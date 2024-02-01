#pragma once

#include <chrono>
#include <mutex>

namespace mavsdk {

typedef std::chrono::time_point<std::chrono::steady_clock> dl_time_t;
typedef std::chrono::time_point<std::chrono::system_clock> dl_system_time_t;
typedef std::chrono::time_point<std::chrono::system_clock> dl_autopilot_time_t;

class Time {
 public:
  Time() = default;
  virtual ~Time() = default;

  virtual dl_time_t steady_time();
  virtual dl_system_time_t system_time();
  double elapsed_s();
  double elapsed_since_s(const dl_time_t& since);
  uint64_t elapsed_ms() const;
  uint64_t elapsed_us() const;
  dl_time_t steady_time_in_future(double duration_s);
  static void shift_steady_time_by(dl_time_t& time, double offset_s);

  virtual void sleep_for(std::chrono::hours h);
  virtual void sleep_for(std::chrono::minutes m);
  virtual void sleep_for(std::chrono::seconds s);
  virtual void sleep_for(std::chrono::milliseconds ms);
  virtual void sleep_for(std::chrono::microseconds us);
  virtual void sleep_for(std::chrono::nanoseconds ns);
};

class FakeTime : public Time {
 public:
  FakeTime();
  ~FakeTime() override = default;
  dl_time_t steady_time() override;
  void sleep_for(std::chrono::hours h) override;
  void sleep_for(std::chrono::minutes m) override;
  void sleep_for(std::chrono::seconds s) override;
  void sleep_for(std::chrono::milliseconds ms) override;
  void sleep_for(std::chrono::microseconds us) override;
  void sleep_for(std::chrono::nanoseconds ns) override;

 private:
  std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds>
      _current{};
  void add_overhead();
};

class AutopilotTime {
 public:
  AutopilotTime() = default;
  virtual ~AutopilotTime() = default;

  dl_autopilot_time_t now();

  void shift_time_by(std::chrono::nanoseconds offset);

  dl_autopilot_time_t time_in(dl_system_time_t local_system_time_point);

 private:
  mutable std::mutex _autopilot_system_time_offset_mutex{};
  std::chrono::nanoseconds _autopilot_time_offset{};

  virtual dl_system_time_t system_time();
};

}  // namespace mavsdk