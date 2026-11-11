# RC range profiles

Air exports `RC_BW_RANGES`, `RC_MCS_RANGES`, `RC_TX_RANGES`, and `RC_FHSS_RANGES`
as PARAM_EXT strings. Each string
is a complete profile of one to five inclusive `upper:value` areas, in increasing
PWM order. Coverage starts at 900 us and must end at 2100 us. Bandwidth values
are 10, 20 and 40 MHz. MCS values are 0 through 4. Transmit mode uses 0 (Off),
1 (Pit), or 2 (Normal), and frequency hopping uses 0 (Off) or 1 (On). Values may repeat or appear in
any order. A rejected profile leaves the previous profile untouched.

Fresh defaults are `1500:20,2100:40`, `1300:0,1700:2,2100:3`,
`1300:0,1700:1,2100:2`, and `1500:0,2100:1`, respectively. Profiles persist
in the normal WBLink settings file.
When an older file already enables the corresponding RC assignment, missing
profiles use the legacy thresholds and values. Disabled assignments get the new
defaults. Invalid persisted profiles recover using the same migration rule.
Transmit mode and frequency hopping retain their original thresholds on upgrade.

Each direct RC function runs when its channel is assigned. `RC_OHD_CTRL`
controls the transmitter's four-channel Lua protocol.

Build and run `test_rc_range_mapping`. It needs only a C++17 compiler and tests
all PWM boundaries, invalid input, arbitrary MCS triples, supported bandwidth
values, transmit and hopping modes, the full five-area MCS profile, and
legacy/default migration selection.
The runtime helper, WBLink registration, and persistence sources must also build.
QOpenHD's `tests/run_joystick_settings_test.py` covers editing and staged writes.
Hardware validation requires installing matching Air firmware and verifying FC
PWM, actual channel width/MCS changes, and persistence across an Air reboot.
