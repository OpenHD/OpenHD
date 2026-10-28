#include "openhd_gui.h"

#include <QApplication>
#include <QEventLoop>
#include <QFile>
#include <QFont>
#include <QLabel>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QScrollBar>
#include <QScroller>
#include <QTemporaryFile>
#include <QTextCursor>
#include <QVBoxLayout>
#include <QWidget>
#include <cstdio>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

namespace openhd::ui {
namespace {
#ifdef _WIN32
int duplicate(int fd) { return _dup(fd); }
void restore(int saved, FILE* stream) { if (saved >= 0) { _dup2(saved, _fileno(stream)); _close(saved); } }
#else
int duplicate(int fd) { return dup(fd); }
void restore(int saved, FILE* stream) { if (saved >= 0) { dup2(saved, fileno(stream)); close(saved); } }
#endif
}
struct Gui::Impl {
  int argc = 1;
  char name[7] = "openhd";
  char* argv[2] = {name, nullptr};
  QApplication app{argc, argv};
  QWidget window;
  QLabel status{"Discovering radios..."};
  QPlainTextEdit radios, diagnostics;
  QTemporaryFile capture;
  QFile reader;
  int saved_out = -1, saved_err = -1;
  QByteArray pending;

  Impl() {
    app.setApplicationName("OpenHD");
#ifdef Q_OS_WIN
    app.setFont(QFont("Segoe UI", 11));
#else
    app.setFont(QFont("sans-serif", 11));
#endif
    app.setQuitOnLastWindowClosed(false);
    window.setWindowTitle("OpenHD - Radio diagnostics");
    auto* layout = new QVBoxLayout(&window);
    layout->addWidget(new QLabel("Detected Devourer / wifibroadcast adapters"));
    radios.setReadOnly(true);
    radios.setMaximumHeight(160);
    radios.setPlainText("Scanning...");
    layout->addWidget(&radios);
    status.setWordWrap(true);
    status.setTextFormat(Qt::PlainText);
    layout->addWidget(&status);
    layout->addWidget(new QLabel("Live diagnostics"));
    diagnostics.setReadOnly(true);
    diagnostics.setMaximumBlockCount(3000);
    layout->addWidget(&diagnostics, 1);
    for (auto* view : {&radios, &diagnostics}) {
      QScroller::grabGesture(view->viewport(), QScroller::TouchGesture);
    }
    window.setStyleSheet("QWidget { background: #18202b; color: #edf2f7; font-size: 16px; font-family: '" +
        app.font().family() + "'; } QPlainTextEdit { background: #101720; }");
    window.resize(1000, 640);
    // Capture C stdio too: Devourer and third-party libraries use printf as
    // well as OpenHD's spdlog/std::cout. A separate reader never moves the
    // writers' file offsets. Bound both disk use and the displayed history.
    if (capture.open()) {
      fflush(stdout); fflush(stderr);
      saved_out = duplicate(fileno(stdout));
      saved_err = duplicate(fileno(stderr));
      const auto path = QFile::encodeName(capture.fileName());
      if (freopen(path.constData(), "ab", stdout)) setvbuf(stdout, nullptr, _IONBF, 0);
      if (freopen(path.constData(), "ab", stderr)) setvbuf(stderr, nullptr, _IONBF, 0);
      reader.setFileName(capture.fileName());
      reader.open(QIODevice::ReadOnly);
    } else {
      diagnostics.setPlainText("Unable to capture runtime diagnostics.");
    }
    if (QApplication::platformName().startsWith("eglfs")) window.showFullScreen();
    else window.show();
  }
  ~Impl() {
    fflush(stdout); fflush(stderr);
    restore(saved_out, stdout); restore(saved_err, stderr);
  }
  void drain_logs() {
    if (!reader.isOpen()) return;
    const bool at_bottom = diagnostics.verticalScrollBar()->value() >= diagnostics.verticalScrollBar()->maximum() - 4;
    pending += reader.read(64 * 1024);
    const int end = pending.lastIndexOf('\n');
    if (end >= 0) {
      auto text = QString::fromUtf8(pending.left(end + 1));
      text.remove(QRegularExpression("\\x1b\\[[0-?]*[ -/]*[@-~]"));
      QTextCursor cursor(diagnostics.document());
      cursor.movePosition(QTextCursor::End);
      cursor.insertText(text);
      pending.remove(0, end + 1);
      if (at_bottom) diagnostics.verticalScrollBar()->setValue(diagnostics.verticalScrollBar()->maximum());
    }
    if (pending.size() > 64 * 1024) pending.clear();
    if (reader.pos() >= 10 * 1024 * 1024) {
      capture.resize(0);
      reader.seek(0);
    }
  }
};
Gui::Gui() {
#ifdef Q_OS_LINUX
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) qputenv("QT_QPA_PLATFORM", "eglfs");
#endif
  impl_ = std::make_unique<Impl>();
}
Gui::~Gui() = default;
bool Gui::process_events() {
  impl_->drain_logs();
  impl_->app.processEvents(QEventLoop::AllEvents);
  return impl_->window.isVisible();
}
void Gui::set_radios(const std::string& details) {
  const auto text = QString::fromStdString(details);
  if (impl_->radios.toPlainText() != text) impl_->radios.setPlainText(text);
}
void Gui::set_status(const std::string& details) {
  impl_->status.setText(QString::fromStdString(details));
}
}  // namespace openhd::ui
