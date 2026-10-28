#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QProgressDialog>
#include <QStandardPaths>
#include <QTimer>
#include <QUuid>
#include <future>

#include "openhd_gui.h"
#include "windows_usb_driver.h"
namespace openhd::ui {
int run_windows_usb_driver_install(int argc, char** argv) {
  QApplication app(argc, argv);
  app.setApplicationName("OpenHD WiFi Test");
  const auto args = app.arguments();
  if (args.size() != 3 || args[1] != "--install-winusb-device") return 2;
  QProgressDialog progress(
      "Installing WinUSB for the selected OpenHD adapter...", QString(), 0, 0);
  progress.setWindowTitle("OpenHD USB driver setup");
  progress.setCancelButton(nullptr);
  progress.setWindowFlag(Qt::WindowCloseButtonHint, false);
  progress.setMinimumDuration(0);
  progress.show();
  const auto instance = args[2].toUtf8().toStdString();
  const auto folder =
      QStandardPaths::writableLocation(QStandardPaths::TempLocation) +
      "/OpenHD-WinUSB-" + QUuid::createUuid().toString(QUuid::Id128);
  const auto path = std::filesystem::path(folder.toStdWString());
  auto work = std::async(std::launch::async, [instance, path] {
    wifi_test::install_windows_usb_driver(instance, path);
  });
  int result = 1;
  QTimer timer;
  QObject::connect(&timer, &QTimer::timeout, [&] {
    if (work.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
      return;
    timer.stop();
    try {
      work.get();
      result = 0;
      QMessageBox::information(
          &progress, "WinUSB installed",
          "WinUSB installation completed. Return to WifiTest; it will refresh "
          "and verify the adapter. "
          "If Windows requests it, unplug and reconnect the adapter.");
    } catch (const std::exception& e) {
      QMessageBox::warning(&progress, "WinUSB installation failed",
                           QString::fromUtf8(e.what()));
    }
    app.quit();
  });
  timer.start(100);
  app.exec();
  return result;
}
}  // namespace openhd::ui
