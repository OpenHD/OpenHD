#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QElapsedTimer>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <iostream>
#include <stdexcept>

#include "openhd_gui.h"
#include "wifi_test.h"
#include "wifi_test_widgets.h"
static void require(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}
int main(int argc, char** argv) {
  try {
    openhd::ui::Gui gui;
    require(gui.process_events(), "GUI failed to open");
    QWidget* window = nullptr;
    for (auto* w : QApplication::topLevelWidgets())
      if (w->windowTitle() == "OpenHD WiFi Test") window = w;
    require(window, "Main test window missing");
    require(!window->windowIcon().pixmap(32, 32).isNull(),
            "OpenHD window icon is missing");
    auto* frequency = window->findChild<QComboBox*>("frequency");
    auto* width = window->findChild<QComboBox*>("width");
    auto* mcs = window->findChild<QComboBox*>("mcs");
    auto* table = window->findChild<QTableWidget*>("events");
    require(frequency && width && mcs && table, "Test controls missing");
    require(frequency->count() == int(openhd::wifi_test::channels().size()),
            "GUI omitted OpenHD channels");
    require(
        mcs->count() == 32 && width->findData(5) < 0 && width->findData(80) < 0,
        "GUI limits differ from OpenHD");
    for (auto* combo : window->findChildren<QComboBox*>())
      require(!combo->isEditable(), "A setting is not a dropdown");
    require(table->rowCount() == 0, "GUI invented test events before Start");
    frequency->setCurrentIndex((frequency->currentIndex() + 1) %
                               frequency->count());
    gui.process_events();
    require(table->rowCount() == 0,
            "Editing a dropdown started a test or invented history");
    QElapsedTimer timer;
    timer.start();
    for (int n = 0; n < 10; ++n)
      require(gui.process_events(), "Window closed unexpectedly");
    require(timer.elapsed() < 2000, "Idle GUI event loop blocked");
    if (argc > 1) {
      const QString directory = QString::fromLocal8Bit(argv[1]);
      QDir().mkpath(directory);
      window->resize(1160, 780);
      gui.process_events();
      window->grab().save(directory + "/wifi-test-desktop.png");
      window->resize(800, 480);
      gui.process_events();
      window->grab().save(directory + "/wifi-test-touch.png");
    }
    window->close();
    require(!gui.process_events(), "Window close did not stop the GUI loop");
    std::cout << "Dropdown limits, empty history, responsive event loop and "
                 "close passed\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
