# OpenHD RC settings for EdgeTX

Copy both `SCRIPTS` subdirectories to the radio SD card. Enable the `OHDSET`
mixer script in the model and route its `D2`, `D1`, `D0`, and `CLK` outputs to
four consecutive, otherwise unused channels. Set OpenHD `RC_OHD_CTRL=1` and
`RC_SET_BASE` to the first channel number. Run the `OpenHD` tool to send a
setting. Each command takes about 1.35 seconds.

The wire frame is a `111` start symbol followed by eight three-bit symbols:
`setting_id:5 | value:11 | sequence:3 | CRC-5:5`. The fourth channel toggles
for each symbol, so decoding does not depend on an exact RC packet rate.

Implemented setting IDs are:

| ID | Setting | Wire value |
|---:|---|---|
| 1 | Frequency | 2.4GHz channel 1..14, otherwise MHz / 5 |
| 2 | Bandwidth | 0=10MHz, 1=20MHz, 2=40MHz |
| 3 | MCS | 0..4 |
| 4 | TX power | 20, 40, 60, 80, or 100 percent |
| 6 | FHSS | 0=off (the EdgeTX tool intentionally exposes disable only) |

Frequency changes use OpenHD's existing managed change path. FHSS remains
limited to the Devourer backend. The EdgeTX action only disables FHSS on Air,
returning it to the selected fixed OpenHD channel; enabling FHSS requires its
normal coordinated setup. IDs 5, 7, 8, 9, and 17 are intentionally not
implemented.
