#include "rc_range_mapping.h"

#include <cassert>
#include <iostream>
#include <string>

using namespace openhd::rc;

int main() {
  for (int pwm = 0; pwm <= 65535; ++pwm) {
    const auto bw = value_for_pwm(default_bw_ranges, RangeKind::Bandwidth, pwm);
    const auto mcs = value_for_pwm(default_mcs_ranges, RangeKind::Mcs, pwm);
    const auto tx = value_for_pwm(default_tx_mode_ranges, RangeKind::TxMode, pwm);
    const auto fhss = value_for_pwm(default_fhss_ranges, RangeKind::Fhss, pwm);
    if (pwm < 900 || pwm > 2100) {
      assert(!bw && !mcs && !tx && !fhss);
    } else {
      assert(bw == (pwm <= 1500 ? 20 : 40));
      assert(mcs == (pwm <= 1300 ? 0 : pwm <= 1700 ? 2 : 3));
      assert(tx == (pwm <= 1300 ? 0 : pwm <= 1700 ? 1 : 2));
      assert(fhss == (pwm <= 1500 ? 0 : 1));
      assert(value_for_pwm(legacy_bw_ranges, RangeKind::Bandwidth, pwm) ==
             (pwm <= 1300 ? 10 : pwm <= 1700 ? 20 : 40));
      assert(value_for_pwm(legacy_mcs_ranges, RangeKind::Mcs, pwm) ==
             (pwm <= 1200 ? 0 : pwm <= 1400 ? 1 : pwm <= 1600 ? 2 : pwm <= 1800 ? 3 : 4));
    }
  }
  // Arbitrary values/order, including the requested 1/3/4 and 0/1/2/3/4 maps.
  for (int first = 0; first <= 4; ++first)
    for (int second = 0; second <= 4; ++second)
      for (int third = 0; third <= 4; ++third) {
        const std::string text = "1100:" + std::to_string(first) + ",1800:" +
            std::to_string(second) + ",2100:" + std::to_string(third);
        for (int pwm = 900; pwm <= 2100; ++pwm)
          assert(value_for_pwm(text, RangeKind::Mcs, pwm) ==
                 (pwm <= 1100 ? first : pwm <= 1800 ? second : third));
      }
  assert(value_for_pwm("1100:1,1800:3,2100:4", RangeKind::Mcs, 1801) == 4);
  assert(value_for_pwm("1100:40,1800:10,2100:20", RangeKind::Bandwidth, 1800) == 10);
  assert(value_for_pwm("2100:20", RangeKind::Bandwidth, 900) == 20);
  assert(value_for_pwm("900:0,2100:4", RangeKind::Mcs, 900) == 0);
  assert(value_for_pwm("900:0,2100:4", RangeKind::Mcs, 901) == 4);
  for (const auto* invalid : {"", "2100", "2100:", ":20", "899:20,2100:40",
       "1500:20,1500:40,2100:10", "1700:20,1300:10,2100:40", "2000:20",
       "2101:20", "2100:80", "2100:-1", "2100:20,", ",2100:20", "2100:20x",
       " 2100:20", "+2100:20", "2100:20:40", "999999999999:20",
       "1000:10,1200:20,1400:40,1600:10,1800:20,2100:40"}) {
    assert(!parse_ranges(invalid, RangeKind::Bandwidth));
  }
  assert(!parse_ranges("2100:5", RangeKind::Mcs));
  assert(!parse_ranges("2100:20", RangeKind::Mcs));
  assert(!parse_ranges("2100:3", RangeKind::TxMode));
  assert(!parse_ranges("2100:2", RangeKind::Fhss));
  assert(value_for_pwm("1250:0,1800:2,2100:1", RangeKind::TxMode, 1500) == 2);
  assert(value_for_pwm("1250:0,1800:1,2100:0", RangeKind::Fhss, 1801) == 0);
  assert(std::string(initial_ranges(RangeKind::Bandwidth, true)) == legacy_bw_ranges);
  assert(std::string(initial_ranges(RangeKind::Mcs, true)) == legacy_mcs_ranges);
  assert(std::string(initial_ranges(RangeKind::Bandwidth, false)) == default_bw_ranges);
  assert(std::string(initial_ranges(RangeKind::Mcs, false)) == default_mcs_ranges);
  std::cout << "RC range mapping: PWM bounds, custom values, defaults, migration and malformed profiles passed\n";
}
