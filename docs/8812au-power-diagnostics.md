# 8812AU power diagnostics

Build the normal OpenHD target and image; package, service and executable names
are unchanged. This adds diagnostics, without replacing EFUSE-relative power
control or changing the requested power levels.

With persistent logging enabled (the same control used for `camera.log`),
`power.log` is saved beside `camera.log` under the recordings directory's
`logs/openhd/` folder. Development images enable persistent logging by default;
otherwise enable it in OpenHD's logging control or use
`/Config/openhd/enable_logs.txt` and restart OpenHD. A `disable_logs.txt` marker
must be removed if present. Logs rotate at 10 MiB with three backups and flush
each diagnostic entry. Disabling persistent logging also disables the extra
power-register reads.

Each Devourer power application records:

- Air/Ground role, card/interface, configured frequency and Air bandwidth,
  selected/effective power level, PIT setting, arming state and RC override.
- Before/after offset, saturation flags and whether the values are hardware
  readback or software state.
- On 8812AU/8821AU, both available RF paths' raw HT MCS0..7 TXAGC words
  (`0xc2c/0xc30`, `0xe2c/0xe30`), training and BB-swing registers, and
  expected versus read-back MCS0/MCS7 indices. Path 0 is A, path 1 is B.

`match=true` means the two sampled rate indices match the software calculation;
it does **not** mean RF power was measured. Other chips retain their existing
representative power-state reporting; the A/B raw snapshot is Jaguar1-only.
Readback exceptions are logged and do not cancel the power change.

## Tester procedure

1. Confirm Air identifies the ASUS USB-AC56 as `0b05:17d2`. Disable Air PIT
   mode and any RC transmit-mode override. Enable persistent logging before
   testing. Confirm `power-diag-v1` appears in `power.log`.
2. Disable FHSS. Keep frequency, bandwidth, MCS, antennas, position and distance
   fixed. Avoid receiver overload; keep antennas separated enough for a useful
   RSSI change.
3. Set **Air** power to **20% -> 60% -> 100% -> 20%**, holding each for about
   ten seconds. Record Ground's received Air RSSI at every level, including
   per-antenna RSSI if available. Leave Ground power unchanged.
4. Return `power.log` and rotated backups, `devourer.log` if available, and
   the RSSI notes. Include board model, frequency, bandwidth, MCS and build ID.

The immediate `after` snapshot checks whether writes landed. The `before`
snapshot at the next change shows whether the preceding setting persisted.
Identical register values with different expected indices point to a write or
clamping problem. Moving, matching indices with flat received RSSI narrows the
problem to the RF path or measurement; it does not establish either cause.
