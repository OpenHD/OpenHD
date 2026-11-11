# RC range profiles

Air exports `RC_BW_RANGES` and `RC_MCS_RANGES` as PARAM_EXT strings. Each string
is a complete profile of one to five inclusive `upper:value` areas, in increasing
PWM order. Coverage starts at 900 us and must end at 2100 us. Bandwidth values
are 10, 20 and 40 MHz; MCS values are 0 through 4. Values may repeat or appear in
any order. A rejected profile leaves the previous profile untouched.

Fresh defaults are `1500:20,2100:40` and `1300:0,1700:2,2100:3`. Profiles persist
in the normal WBLink settings file as `wb_bw_rc_ranges` and `wb_mcs_rc_ranges`.
When an older file already enables the corresponding RC assignment, missing
profiles use the legacy thresholds and values. Disabled assignments get the new
defaults. Invalid persisted profiles recover using the same migration rule.

Build and run `test_rc_range_mapping`. It needs only a C++17 compiler and tests
all PWM boundaries, invalid input, arbitrary MCS triples, supported bandwidth
values, the full five-area MCS profile, and legacy/default migration selection.
The runtime helper, WBLink registration, and persistence sources must also build.
QOpenHD's `tests/run_joystick_settings_test.py` covers editing and staged writes.
Hardware validation requires installing matching Air firmware and verifying FC
PWM, actual channel width/MCS changes, and persistence across an Air reboot.
