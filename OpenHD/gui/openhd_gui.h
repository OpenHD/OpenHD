#pragma once

#include <memory>
#include <string>

namespace openhd::ui {

// Own on the main thread, alongside OpenHD's interface/telemetry/video objects.
// Future controls can receive those objects or callbacks directly in-process.
class Gui {
 public:
  Gui();
  ~Gui();
  Gui(const Gui&) = delete;
  Gui& operator=(const Gui&) = delete;

  // Service Qt from the existing main loop. False means the window was closed.
  bool process_events();
  void set_radios(const std::string& details);
  void set_status(const std::string& details);

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace openhd::ui
