#include "openhd_gui.h"

#include <QApplication>
#include <QComboBox>
#include <QDateTime>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QScrollBar>
#include <QScroller>
#include <QSignalBlocker>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QTableWidget>
#include <QTemporaryFile>
#include <QTextCursor>
#include <QTimer>
#include <QUuid>
#include <QVBoxLayout>
#include <csignal>
#include <cstdio>
#include <future>
#include <thread>

#include "wifi_test_widgets.h"
static void initialize_gui_resources() { Q_INIT_RESOURCE(wifi_test); }
#ifdef _WIN32
#include <io.h>
#include <shellapi.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace openhd::ui {
namespace {
#ifdef _WIN32
int duplicate(int fd) { return _dup(fd); }
void restore(int fd, FILE* stream) {
  if (fd >= 0) {
    _dup2(fd, _fileno(stream));
    _close(fd);
  }
}
std::filesystem::path path_from_qt(const QString& value) {
  return value.toStdWString();
}
#else
int duplicate(int fd) { return dup(fd); }
void restore(int fd, FILE* stream) {
  if (fd >= 0) {
    dup2(fd, fileno(stream));
    close(fd);
  }
}
std::filesystem::path path_from_qt(const QString& value) {
  return value.toStdString();
}
#endif
void select_value(QComboBox& combo, int value) {
  const int index = combo.findData(value);
  if (index >= 0) combo.setCurrentIndex(index);
}
void integers(QComboBox& combo, int first, int last, int step = 1,
              const QString& suffix = {}) {
  for (int value = first; value <= last; value += step)
    combo.addItem(QString::number(value) + suffix, value);
}
}  // namespace
struct Gui::Impl {
  int argc = 1;
  char name[7] = "openhd";
  char* argv[2] = {name, nullptr};
  QApplication app{argc, argv};
  QWidget window;
  QLabel status, metrics, session_path, adapter_notice;
  TestDropdown device, frequency, width, mcs, power, stbc, ldpc, gi, beam,
      payload, fec, block, rate, packet, seed, ack, view;
  TestControlPanels* panels = nullptr;
  TestControlPanels* advanced = nullptr;
  QPushButton start{"Start"}, apply{"Apply settings"},
      refresh{"Refresh adapters"}, setup_driver{"Set up WinUSB..."},
      export_data{"Export session"};
  TestTimeline timeline;
  QTableWidget table;
  QPlainTextEdit diagnostics;
  QTemporaryFile capture;
  QFile reader;
  QFile session_log;
  QByteArray pending_session_log;
  int saved_out = -1, saved_err = -1;
  QByteArray pending;
  QTimer poll_timer;
  std::unique_ptr<wifi_test::Controller> controller;
  std::future<wifi_test::Json> device_scan;
  QVariant previous_device;
  wifi_test::Json last_events = wifi_test::Json::array();
  QString directory;
  bool is_running = false;
#ifdef _WIN32
  HANDLE driver_process = nullptr;
#endif

  Impl() {
    initialize_gui_resources();
    app.setApplicationName("OpenHD WiFi Test");
    app.setWindowIcon(QIcon(":/openhd/openhd.ico"));
    app.setQuitOnLastWindowClosed(false);
#ifdef Q_OS_WIN
    app.setFont(QFont("Segoe UI", 10));
#else
    app.setFont(QFont("sans-serif", 10));
#endif
    window.setWindowTitle("OpenHD WiFi Test");
    window.resize(1160, 780);
    auto* outer = new QVBoxLayout(&window);
    outer->setContentsMargins(0, 0, 0, 0);
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    outer->addWidget(scroll);
    auto* content = new QWidget;
    scroll->setWidget(content);
    QScroller::grabGesture(scroll->viewport(), QScroller::TouchGesture);
    auto* root = new QVBoxLayout(content);
    root->setSizeConstraint(QLayout::SetNoConstraint);
    root->setContentsMargins(16, 12, 16, 12);
    root->setSpacing(10);
    auto* heading = new QHBoxLayout;
    auto* logo = new QLabel;
    logo->setPixmap(app.windowIcon().pixmap(32, 32));
    logo->setFixedSize(32, 32);
    heading->addWidget(logo);
    auto* title = new QLabel("OpenHD WiFi Test");
    title->setStyleSheet("font-size:23px;font-weight:600;");
    heading->addWidget(title);
    heading->addStretch();
    heading->addWidget(&export_data);
    root->addLayout(heading);
    auto* adapter_row = new QHBoxLayout;
    adapter_row->addWidget(new QLabel("Adapter"));
    adapter_row->addWidget(&device, 1);
    adapter_row->addWidget(&refresh);
    adapter_row->addWidget(&setup_driver);
    setup_driver.setObjectName("setupWinUSB");
#ifndef _WIN32
    setup_driver.hide();
#endif
    root->addLayout(adapter_row);
    adapter_notice.setWordWrap(true);
    adapter_notice.setTextFormat(Qt::PlainText);
    adapter_notice.setObjectName("adapterNotice");
    root->addWidget(&adapter_notice);
    device.setObjectName("adapter");
    frequency.setObjectName("frequency");
    width.setObjectName("width");
    mcs.setObjectName("mcs");
    power.setObjectName("power");
    payload.setObjectName("payload");
    start.setObjectName("startStop");
    apply.setObjectName("apply");
    table.setObjectName("events");
    diagnostics.setObjectName("diagnostics");
    for (const auto& c : wifi_test::channels()) {
      const int mhz = c.at("mhz"), channel = c.at("channel");
      frequency.addItem(
          QString::number(mhz) + " MHz" +
              (channel >= 0 ? " (ch " + QString::number(channel) + ")"
                            : " (extended)"),
          mhz);
    }
    const auto options = wifi_test::control_options();
    for (const auto& n : options.at("width"))
      width.addItem(QString::number(n.get<int>()) + " MHz", n.get<int>());
    for (const auto& n : options.at("mcs"))
      mcs.addItem(QString::number(n.get<int>()), n.get<int>());
    for (const auto& n : options.at("power")) {
      int p = n;
      power.addItem(QString::number(p) + "%" + (p > 100 ? " overdrive" : ""),
                    p);
    }
    for (const auto& n : options.at("stbc"))
      stbc.addItem(n == 0 ? "Off" : QString::number(n.get<int>()),
                   n.get<int>());
    for (auto* combo : {&ldpc, &gi}) {
      combo->addItem("Off", 0);
      combo->addItem("On", 1);
    }
    beam.addItem("Unavailable", 0);
    beam.setToolTip(
        "The shared OpenHD transmit path does not expose beamforming");
    beam.setEnabled(false);
    payload.addItem("Garbage", 0);
    payload.addItem("Sample video (H.264)", 1);
    for (const auto& n : options.at("fec"))
      fec.addItem(n == 0 ? "Off" : QString::number(n.get<int>()) + "%",
                  n.get<int>());
    for (const auto& n : options.at("block"))
      block.addItem(n == 0 ? "Auto" : QString::number(n.get<int>()),
                    n.get<int>());
    integers(rate, 100, 100000, 100, " kbit/s");
    integers(packet, 64, 1441, 1, " bytes");
    for (int value : {1, 42, 12345, 314159})
      seed.addItem(QString::number(value), value);
    ack.addItem("Air default (no ACK)", 1);
    ack.addItem("High retry flag", 0);
    for (auto* combo :
         {&frequency, &width, &mcs, &power, &stbc, &ldpc, &gi, &beam, &payload,
          &fec, &block, &rate, &packet, &seed, &ack}) {
      combo->setSizeAdjustPolicy(
          QComboBox::AdjustToMinimumContentsLengthWithIcon);
      combo->setMinimumContentsLength(3);
      combo->setMaxVisibleItems(18);
    }
    select_value(frequency, 5745);
    select_value(width, 20);
    select_value(mcs, 2);
    select_value(power, 20);
    select_value(stbc, 1);
    select_value(ldpc, 1);
    select_value(gi, 0);
    select_value(fec, 20);
    select_value(rate, 4000);
    select_value(packet, 1024);
    select_value(seed, 42);
    panels = new TestControlPanels;
    auto panel = [&](const QString& name, QComboBox& combo) {
      auto* group = new QGroupBox(name);
      auto* layout = new QVBoxLayout(group);
      layout->addWidget(&combo);
      layout->addStretch();
      panels->add(group);
    };
    panel("Freq", frequency);
    panel("Width", width);
    panel("MCS", mcs);
    panel("Power", power);
    auto* flags = new QGroupBox("PHY flags");
    auto* flags_layout = new QFormLayout(flags);
    flags_layout->addRow("STBC", &stbc);
    flags_layout->addRow("LDPC", &ldpc);
    flags_layout->addRow("Short GI", &gi);
    flags_layout->addRow("Beamforming", &beam);
    panels->add(flags);
    panel("Payload", payload);
    auto* actions = new QWidget;
    auto* action_layout = new QVBoxLayout(actions);
    start.setFixedHeight(96);
    start.setProperty("primary", true);
    action_layout->addWidget(&start);
    action_layout->addWidget(&apply);
    panels->add(actions);
    root->addWidget(panels);
    advanced = new TestControlPanels;
    auto field = [&](const QString& label, QComboBox& combo) {
      auto* field_widget = new QWidget;
      auto* column = new QVBoxLayout(field_widget);
      column->setContentsMargins(0, 0, 0, 0);
      column->addWidget(new QLabel(label));
      column->addWidget(&combo);
      advanced->add(field_widget);
    };
    field("FEC", fec);
    field("FEC block", block);
    field("Source rate", rate);
    field("Packet / RTP MTU", packet);
    field("Garbage seed", seed);
    field("TX flag", ack);
    root->addWidget(advanced);
    status.setWordWrap(true);
    status.setTextFormat(Qt::PlainText);
    status.setObjectName("status");
    root->addWidget(&status);
    metrics.setWordWrap(true);
    metrics.setTextFormat(Qt::PlainText);
    metrics.setObjectName("metrics");
    root->addWidget(&metrics);
    auto* timeline_heading = new QHBoxLayout;
    timeline_heading->addWidget(new QLabel("Timeline — actual user actions"));
    timeline_heading->addStretch();
    view.addItems({"Timeline and events", "Diagnostics", "Both"});
    timeline_heading->addWidget(&view);
    root->addLayout(timeline_heading);
    root->addWidget(&timeline);
    table.setColumnCount(12);
    table.setHorizontalHeaderLabels({"Time", "Action", "Frequency", "Width",
                                     "MCS", "Power", "STBC", "LDPC", "GI",
                                     "FEC", "Payload", "Result"});
    table.setMinimumHeight(160);
    diagnostics.setMinimumHeight(160);
    table.setEditTriggers(QAbstractItemView::NoEditTriggers);
    table.setSelectionBehavior(QAbstractItemView::SelectRows);
    table.setAlternatingRowColors(true);
    table.verticalHeader()->hide();
    table.horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);
    table.horizontalHeader()->setStretchLastSection(true);
    root->addWidget(&table, 1);
    diagnostics.setReadOnly(true);
    diagnostics.setMaximumBlockCount(3000);
    diagnostics.hide();
    root->addWidget(&diagnostics, 1);
    for (auto* area : {table.viewport(), diagnostics.viewport()})
      QScroller::grabGesture(area, QScroller::TouchGesture);
    session_path.setTextFormat(Qt::PlainText);
    session_path.setWordWrap(true);
    session_path.setStyleSheet("color:#88a1b5;font-size:11px;");
    root->addWidget(&session_path);
    window.setStyleSheet(
        "QWidget{background:#0b151e;color:#dce8f0;font-family:'" +
        app.font().family() +
        "';font-size:14px;}"
        "QGroupBox{background:#111f2b;border:1px solid "
        "#243746;border-radius:8px;margin-top:8px;padding:12px 6px 8px;}"
        "QGroupBox::title{subcontrol-origin:margin;left:10px;color:#9fb4c4;}"
        "QComboBox{background:#142633;border:1px solid "
        "#2b4354;border-radius:5px;min-height:32px;padding:0 6px;}"
        "QComboBox::drop-down{width:20px;border:0;}QComboBox "
        "QAbstractItemView{selection-background-color:#17649c;min-width:130px;}"
        "QPushButton{background:#172b3b;border:1px solid "
        "#2b465b;border-radius:6px;min-height:32px;padding:0 12px;}"
        "QPushButton[primary=true]{background:#0ab75d;border-color:#15cc70;"
        "font-size:19px;font-weight:600;}"
        "QPushButton:disabled{color:#526776;background:#12202b;}QComboBox:"
        "disabled{color:#526776;}"
        "QTableWidget{border:1px solid "
        "#203342;background:#0e1c27;alternate-background-color:#11222f;"
        "gridline-color:#1b2c38;}"
        "QHeaderView::section{background:#10202c;color:#91acc1;border:0;"
        "padding:9px;}QPlainTextEdit{background:#0b121a;border:1px solid "
        "#223744;}"
        "QScrollBar:vertical{background:#0e1e29;width:12px;}QScrollBar::handle:"
        "vertical{background:#31516a;border-radius:5px;min-height:30px;}");
    capture_logs();
    controller = std::make_unique<wifi_test::Controller>();
    QObject::connect(&setup_driver, &QPushButton::clicked,
                     [this] { driver_setup(); });
    QObject::connect(&refresh, &QPushButton::clicked, [this] { scan(); });
    QObject::connect(&start, &QPushButton::clicked, [this] {
      try {
        const auto phase = controller->snapshot().value("state", "");
        if (phase == "running" || phase == "starting" || phase == "applying") {
          controller->stop();
          poll();
          return;
        }
        if (phase == "stopping") return;
        auto settings = selection();
        wifi_test::validate(settings);
        session_log.close();
        pending_session_log.clear();
        pending.clear();
        diagnostics.clear();
        reader.seek(capture.size());
        directory =
            QStandardPaths::writableLocation(
                QStandardPaths::AppLocalDataLocation) +
            "/wifi-tests/" +
            QDateTime::currentDateTimeUtc().toString("yyyyMMdd-HHmmss-zzz") +
            "-" + QUuid::createUuid().toString(QUuid::Id128).left(8);
        controller->start(settings, path_from_qt(directory));
        poll();
      } catch (const std::exception& e) {
        status.setText(QString::fromUtf8(e.what()));
      }
    });
    QObject::connect(&apply, &QPushButton::clicked, [this] {
      try {
        auto s = selection();
        wifi_test::validate(s);
        controller->apply(s);
        poll();
      } catch (const std::exception& e) {
        status.setText(e.what());
      }
    });
    QObject::connect(&export_data, &QPushButton::clicked,
                     [this] { export_session(); });
    QObject::connect(&view, QOverload<int>::of(&QComboBox::currentIndexChanged),
                     [this](int mode) {
                       timeline.setVisible(mode != 1);
                       table.setVisible(mode != 1);
                       diagnostics.setVisible(mode != 0);
                     });
    QObject::connect(
        &payload, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int n) {
          seed.setEnabled(n == 0);
          for (int i = 0; i < packet.count(); ++i)
            static_cast<QStandardItemModel*>(packet.model())
                ->item(i)
                ->setEnabled(n == 0 || packet.itemData(i).toInt() >= 564);
          if (n == 1 && packet.currentData().toInt() < 564)
            select_value(packet, 564);
        });
    QObject::connect(&poll_timer, &QTimer::timeout, [this] { poll(); });
    poll_timer.start(100);
    scan();
    poll();
    if (QApplication::platformName().startsWith("eglfs"))
      window.showFullScreen();
    else
      window.show();
  }
  ~Impl() {
#ifdef _WIN32
    if (driver_process) CloseHandle(driver_process);
#endif
    controller.reset();
    drain_logs();
    fflush(stdout);
    fflush(stderr);
    restore(saved_out, stdout);
    restore(saved_err, stderr);
  }
  wifi_test::Settings selection() const {
    wifi_test::Settings s;
    if (!device.currentData(Qt::UserRole + 2).toBool())
      throw std::runtime_error(
          "Adapter is not ready: complete USB driver setup, then refresh "
          "adapters");
    s.device = device.currentData().toString().toStdString();
    s.frequency = frequency.currentData().toInt();
    s.width = width.currentData().toInt();
    s.mcs = mcs.currentData().toInt();
    s.power = power.currentData().toInt();
    s.stbc = stbc.currentData().toInt();
    s.ldpc = ldpc.currentData().toInt();
    s.short_gi = gi.currentData().toInt();
    s.fec = fec.currentData().toInt();
    s.block = block.currentData().toInt();
    s.bitrate_kbps = rate.currentData().toInt();
    s.packet_bytes = packet.currentData().toInt();
    s.seed = seed.currentData().toUInt();
    s.video = payload.currentData().toInt();
    s.no_ack = ack.currentData().toInt();
    return s;
  }
  void driver_setup() {
#ifdef _WIN32
    if (!setup_driver.isEnabled()) return;
    const auto record = wifi_test::Json::parse(
        device.currentData(Qt::UserRole + 1).toString().toStdString());
    const auto vid =
        QString::number(record.at("vid").get<int>(), 16).rightJustified(4, '0');
    const auto pid =
        QString::number(record.at("pid").get<int>(), 16).rightJustified(4, '0');
    QMessageBox guide(&window);
    guide.setWindowTitle("Install WinUSB for Devourer");
    guide.setTextFormat(Qt::PlainText);
    guide.setText("Adapter: " + device.currentText() + "\nUSB ID: " + vid +
                  ":" + pid + "\nCurrent driver: " +
                  QString::fromStdString(record.value("driver", "Unknown")) +
                  "\n\nInstall WinUSB for this supported adapter? "
                  "It will stop working as a normal Windows Wi-Fi connection. "
                  "Device Manager can restore its vendor driver later. "
                  "Windows may use the same driver for identical adapters."
                  "\n\nWindows will ask for administrator approval. "
                  "After installation, the tool will refresh and verify the "
                  "adapter automatically.");
    auto* install = guide.addButton("Install WinUSB", QMessageBox::AcceptRole);
    guide.addButton(QMessageBox::Cancel);
    guide.setDefaultButton(QMessageBox::Cancel);
    guide.exec();
    if (guide.clickedButton() != install) return;
    const auto instance =
        QString::fromStdString(record.at("instance_id").get<std::string>());
    if (instance.contains('"')) return;
    const auto parameters =
        ("--install-winusb-device \"" + instance + "\"").toStdWString();
    const auto executable =
        QDir::toNativeSeparators(QCoreApplication::applicationFilePath())
            .toStdWString();
    SHELLEXECUTEINFOW launch{};
    launch.cbSize = sizeof(launch);
    launch.fMask = SEE_MASK_NOCLOSEPROCESS;
    launch.hwnd = reinterpret_cast<HWND>(window.winId());
    launch.lpVerb = L"runas";
    launch.lpFile = executable.c_str();
    launch.lpParameters = parameters.c_str();
    launch.nShow = SW_SHOWNORMAL;
    if (!ShellExecuteExW(&launch)) {
      QMessageBox::warning(&window, "Driver setup not started",
                           "Administrator approval was cancelled or the "
                           "installer could not start. You can try again.");
      return;
    }
    driver_process = launch.hProcess;
#endif
  }
  void scan() {
    if (device_scan.valid()) return;
    previous_device = device.currentData();
    QSignalBlocker blocker(device);
    device.clear();
    device.addItem("Identifying Devourer Wi-Fi adapters...", QString());
    device_scan =
        std::async(std::launch::async, [] { return wifi_test::devices(); });
  }
  void poll() {
    bool driver_busy = false;
#ifdef _WIN32
    if (driver_process) {
      DWORD exit_code = STILL_ACTIVE;
      if (GetExitCodeProcess(driver_process, &exit_code) &&
          exit_code != STILL_ACTIVE) {
        CloseHandle(driver_process);
        driver_process = nullptr;
        diagnostics.appendPlainText(
            exit_code == 0
                ? "WinUSB setup completed; verifying adapter..."
                : "WinUSB setup did not complete. See installer result.");
        scan();
      } else
        driver_busy = true;
    }
#endif
    if (device_scan.valid() && device_scan.wait_for(std::chrono::seconds(0)) ==
                                   std::future_status::ready) {
      QSignalBlocker blocker(device);
      device.clear();
      try {
        for (const auto& d : device_scan.get()) {
          device.addItem(
              QString::fromStdString(d.at("label").get<std::string>()),
              QString::fromStdString(d.at("selector").get<std::string>()));
          device.setItemData(device.count() - 1,
                             QString::fromStdString(d.dump()),
                             Qt::UserRole + 1);
          device.setItemData(device.count() - 1, d.value("ready", false),
                             Qt::UserRole + 2);
          device.setItemData(device.count() - 1,
                             QString::fromStdString(d.dump(2)),
                             Qt::ToolTipRole);
        }
      } catch (const std::exception& e) {
        diagnostics.appendPlainText(e.what());
      }
      const int found = device.findData(previous_device);
      if (found >= 0) device.setCurrentIndex(found);
      if (device.count() == 0)
        device.addItem(
            "No supported Devourer adapter accessible (check WinUSB)",
            QString());
    }
    panels->arrange(std::max(320, window.width() - 48));
    advanced->arrange(std::max(320, window.width() - 48));
    const auto snapshot = controller->snapshot();
    const auto phase = snapshot.value("state", "");
    if (!session_log.isOpen() && !directory.isEmpty() &&
        QDir(directory).exists()) {
      session_log.setFileName(directory + "/diagnostics.log");
      session_log.open(QIODevice::WriteOnly | QIODevice::Append);
    }
    drain_logs();
    is_running = phase == "running";
    const bool busy = phase == "starting" || phase == "applying" ||
                      phase == "stopping" || driver_busy;
    status.setText(
        QString::fromStdString(snapshot.value("message", std::string())));
    start.setText(phase == "stopping"    ? "Stopping..."
                  : (is_running || busy) ? "Stop"
                                         : "Start");
    if (driver_busy) start.setText("Installing driver...");
    start.setEnabled(
        !driver_busy && phase != "stopping" &&
        (is_running || busy || device.currentData(Qt::UserRole + 2).toBool()));
    apply.setEnabled(is_running);
    device.setEnabled(!is_running && !busy && !device_scan.valid());
    refresh.setEnabled(!is_running && !busy && !device_scan.valid());
    const bool pending_setup =
        !device.currentData(Qt::UserRole + 2).toBool() &&
        !device.currentData(Qt::UserRole + 1).toString().isEmpty();
    setup_driver.setEnabled(pending_setup && !is_running && !busy &&
                            !device_scan.valid());
    adapter_notice.setVisible(pending_setup);
    adapter_notice.setText(
        "Matching USB Wi-Fi adapter detected. WinUSB access is required for "
        "Devourer. "
        "Use Set up WinUSB to install the driver in this tool.");
    export_data.setEnabled(!busy && !is_running &&
                           snapshot.contains("directory"));
    if (snapshot.contains("sample")) {
      const auto& s = snapshot.at("sample");
      metrics.setText(QString("TX %1 packets  ·  %2 Mbit/s  ·  queue rejected "
                              "%3  ·  TX drops %4  ·  late pacing %5")
                          .arg(s.value("tx_packets", int64_t(0)))
                          .arg(s.value("tx_bps", 0) / 1000000.0, 0, 'f', 2)
                          .arg(s.value("rejected", uint64_t(0)))
                          .arg(s.value("tx_drop", 0))
                          .arg(s.value("pacing_late", uint64_t(0))));
      if (snapshot.contains("capabilities"))
        metrics.setText(
            metrics.text() + "\n" +
            QString::fromStdString(
                snapshot.at("capabilities").value("chip", std::string())) +
            QString(" | applied offset %1 dB | thermal raw %2")
                .arg(s.at("applied").value("power_offset_qdb", 0) / 4.0, 0, 'f',
                     2)
                .arg(QString::fromStdString(s.at("thermal_raw").dump())));
      metrics.setToolTip(
          QString::fromStdString(s.value("power_diagnostics", std::string())));
    }
    if (snapshot.contains("directory")) {
      directory =
          QString::fromStdString(snapshot.at("directory").get<std::string>());
      session_path.setText("Session: " + directory);
    }
    const auto& events = snapshot.at("events");
    if (events != last_events) {
      last_events = events;
      update_events();
    }
    timeline.events = events;
    timeline.elapsed_us = snapshot.value("elapsed_us", int64_t(0));
    if (!events.empty())
      timeline.elapsed_us = std::max<qint64>(
          timeline.elapsed_us, events.back().value("elapsed_us", int64_t(0)));
    timeline.update();
  }
  void update_events() {
    table.setRowCount(0);
    for (const auto& e : last_events) {
      const auto type = e.value("type", "");
      if (type != "applied" && type != "stop" && type != "error" &&
          type != "apply_requested")
        continue;
      const auto& s =
          type == "apply_requested" ? e.at("requested") : e.at("settings");
      const int row = table.rowCount();
      table.insertRow(row);
      const QStringList cells = {
          elapsed_text(e.value("elapsed_us", int64_t(0))),
          QString::fromStdString(type),
          QString::number(s.value("frequency_mhz", 0)),
          QString::number(s.value("width_mhz", 0)) + " MHz",
          QString::number(s.value("mcs", 0)),
          QString::number(s.value("power_level_percent", 0)) + "%",
          QString::number(s.value("stbc", 0)),
          s.value("ldpc", false) ? "On" : "Off",
          s.value("short_gi", false) ? "Short" : "Long",
          QString::number(s.value("fec_percent", 0)) + "%",
          QString::fromStdString(s.value("payload", std::string())),
          QString::fromStdString(e.value(
              "message",
              e.value("reason", type == "applied" ? "Backend accepted" : "")))};
      for (int col = 0; col < cells.size(); ++col) {
        auto* item = new QTableWidgetItem(cells[col]);
        if (col == 0) item->setForeground(segment_color(e.value("segment", 0)));
        table.setItem(row, col, item);
      }
    }
    table.scrollToBottom();
  }
  void export_session() {
    const auto destination =
        QFileDialog::getExistingDirectory(&window, "Export test session");
    if (destination.isEmpty()) return;
    try {
      const auto folder = path_from_qt(destination) /
                          path_from_qt(QFileInfo(directory).fileName());
      if (std::filesystem::exists(folder))
        throw std::runtime_error(
            "A session with this name already exists in the destination");
      std::filesystem::create_directories(folder);
      session_log.flush();
      for (const auto& name :
           {"session.jsonl", "samples.csv", "diagnostics.log"})
        std::filesystem::copy_file(path_from_qt(directory) / name,
                                   folder / name);
      status.setText("Session exported to " + destination);
    } catch (const std::exception& e) {
      QMessageBox::warning(&window, "Export failed", e.what());
    }
  }
  void capture_logs() {
    if (!capture.open()) {
      diagnostics.setPlainText("Unable to capture runtime diagnostics.");
      return;
    }
    fflush(stdout);
    fflush(stderr);
    saved_out = duplicate(fileno(stdout));
    saved_err = duplicate(fileno(stderr));
    const auto path = QFile::encodeName(capture.fileName());
    if (freopen(path.constData(), "ab", stdout))
      setvbuf(stdout, nullptr, _IONBF, 0);
    if (freopen(path.constData(), "ab", stderr))
      setvbuf(stderr, nullptr, _IONBF, 0);
    reader.setFileName(capture.fileName());
    reader.open(QIODevice::ReadOnly);
  }
  void drain_logs() {
    if (!reader.isOpen()) return;
    const bool bottom = diagnostics.verticalScrollBar()->value() >=
                        diagnostics.verticalScrollBar()->maximum() - 4;
    pending += reader.read(64 * 1024);
    const int end = pending.lastIndexOf('\n');
    if (end >= 0) {
      auto text = QString::fromUtf8(pending.left(end + 1));
      text.remove(QRegularExpression("\\x1b\\[[0-?]*[ -/]*[@-~]"));
      QTextCursor cursor(diagnostics.document());
      cursor.movePosition(QTextCursor::End);
      cursor.insertText(text);
      pending.remove(0, end + 1);
      pending_session_log += text.toUtf8();
      if (bottom)
        diagnostics.verticalScrollBar()->setValue(
            diagnostics.verticalScrollBar()->maximum());
    }
    if (session_log.isOpen() && !pending_session_log.isEmpty()) {
      if (session_log.write(pending_session_log) ==
              pending_session_log.size() &&
          session_log.flush())
        pending_session_log.clear();
      else {
        if (controller) controller->stop();
        session_log.close();
        pending_session_log.clear();
        status.setText("Diagnostics file write failed; stopping test");
      }
    }
    if (pending_session_log.size() > 1024 * 1024) pending_session_log.clear();
    if (pending.size() > 64 * 1024) pending.clear();
    if (reader.pos() >= 10 * 1024 * 1024) {
      capture.resize(0);
      reader.seek(0);
    }
  }
};
Gui::Gui() {
#ifdef Q_OS_LINUX
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
    qputenv("QT_QPA_PLATFORM", "eglfs");
#endif
  impl_ = std::make_unique<Impl>();
}
Gui::~Gui() = default;
bool Gui::process_events() {
  impl_->poll();
  impl_->app.processEvents(QEventLoop::AllEvents);
  return impl_->window.isVisible();
}
int run_wifi_test_gui() {
  static volatile std::sig_atomic_t stopping = 0;
  stopping = 0;
  const auto old_int = std::signal(SIGINT, [](int) { stopping = 1; });
  const auto old_term = std::signal(SIGTERM, [](int) { stopping = 1; });
  Gui gui;
  while (!stopping && gui.process_events())
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  std::signal(SIGINT, old_int);
  std::signal(SIGTERM, old_term);
  return 0;
}
}  // namespace openhd::ui
