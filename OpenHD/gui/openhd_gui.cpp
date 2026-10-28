#include "openhd_gui.h"

#include <QApplication>
#include <QEventLoop>
#include <QFont>
#include <QLabel>

namespace openhd::ui {

struct Gui::Impl {
  // Qt may modify argc/argv and requires them to outlive QApplication. Keep
  // OpenHD's real arguments untouched for its own parser and restart logic.
  int argc = 1;
  char name[7] = "openhd";
  char* argv[2] = {name, nullptr};
  QApplication app{argc, argv};
  QLabel window{"Hello World"};

  Impl() {
    app.setApplicationName("OpenHD");
    app.setQuitOnLastWindowClosed(false);
    window.setWindowTitle("OpenHD");
    window.setAlignment(Qt::AlignCenter);
    window.setFont(QFont("sans-serif", 32));
    window.setStyleSheet("QLabel { background: #18202b; color: white; }");
    window.resize(800, 480);
    // EGLFS handles touch through Qt's libinput/evdev backend. Widgets also
    // accept Qt's synthesized mouse events from a primary touch contact.
    if (QApplication::platformName().startsWith("eglfs")) {
      window.showFullScreen();
    } else {
      window.show();
    }
  }
};

Gui::Gui() {
#ifdef Q_OS_LINUX
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
    qputenv("QT_QPA_PLATFORM", "eglfs");
  }
#endif
  impl_ = std::make_unique<Impl>();
}
Gui::~Gui() = default;

bool Gui::process_events() {
  impl_->app.processEvents(QEventLoop::AllEvents);
  return impl_->window.isVisible();
}

}  // namespace openhd::ui
