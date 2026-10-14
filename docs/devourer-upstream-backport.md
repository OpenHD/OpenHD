# Devourer upstream backport

Source review: 2026-10-14. This backport merges OpenIPC Devourer `master`
through [`74c6b37`](https://github.com/OpenIPC/devourer/commit/74c6b37)
into the OpenHD driver based on `9e04fca`. It includes all 12 upstream
commits missing from that pin. The resulting driver revision is `e9a8486`
on branch `codex/backport-openipc-20261014`; it also includes the
current OpenHD runtime branch ancestry (`22702c1`). OpenHD consumes the
merged driver through `OpenHD/ohd_interface/lib/devourer`.

## Changes

- Jaguar3: prevent RTL8822C wide-frame rate-fallback stalls, and apply the
  upstream configurable wide-transmit DIG floor to RTL8822C and RTL8822E.
- MT7612U: retain queued USB transfers instead of cancelling them on a
  transfer timeout; refuse new submissions when the ring stays full.
- Station mode: broader station-client coverage, AP restart recovery,
  managed MT7612U receive filtering, and teardown fixes.
- Timing: per-frame stream measurements, hardware-clocked markers, CCX
  report matching, live SVC input, and corrected duplex startup/shutdown.
- PCIe: RTL8852CE VFIO transport, hardware-clock discipline, and host LTR
  capability handling.

OpenHD FHSS, silicon probing, firmware-transfer fixes, primary-20-MHz
mapping, USB TX diagnostics, and old vendor-library fallbacks are retained.
The fork's `.github/workflows` remain unchanged.

## Integration boundaries

The normal OpenHD build still disables Devourer PCIe and leaves the
optional MT7612U backend disabled. Their upstream implementation is
included and checked in the standalone driver build. Stream timing is
available in upstream tools and APIs; this backport does not add timing
fields to OpenHD's MAVLink statistics or alter its video packet format.

The final upstream source differs from the earlier text in its squash
commit message: `DeviceConfig::Tuning::wide_tx_igi_floor` defaults to
`0x35` and applies to both Jaguar3 dies. A session's first transmitted
40/80 MHz frame arms it; subsequent retunes apply it at wide widths and
remove it at narrower widths. Receive-only sessions and sessions sending
only 20 MHz frames do not arm it. The workaround can reduce idle-channel
receive sensitivity on a device transmitting wide frames. Upstream's
sensitivity estimate is not a measured range result. The final source
and `docs/experiments/8822c-ht40-tx-stall.md` are the reference, rather
than the initial commit-message measurements.

## Validation

Ubuntu 22.04 / GCC 11.4 standalone validation enabled all default Realtek
backends, MT7612U, PCIe, and required OpenSSL station crypto tests:

```sh
cmake -S OpenHD/ohd_interface/lib/devourer -B /tmp/devourer-build -G Ninja \
  -DDEVOURER_MT7612U=ON -DDEVOURER_PCIE=ON \
  -DDEVOURER_REQUIRE_STA_CRYPTO_TESTS=ON
cmake --build /tmp/devourer-build --parallel 8
ctest --test-dir /tmp/devourer-build --output-on-failure --parallel 6
```

All 387 build steps succeeded. Of 90 registered tests, 88 passed and two
intentionally skipped optional MT76 reference-data checks
(`mt7612u_usb_ids_vs_mt76`, `mt7612u_initvals_generated`). FHSS lockstep,
stream telemetry, station/crypto, and native/fallback packet-view and
cooperative-thread tests passed.

OpenHD integration validation used a separate Debug build with
`OPENHD_ENABLE_DEVOURER=ON`. All 212 build steps succeeded for
`OHDInterfaceLib`, its Wifibroadcast/Devourer transport dependency,
`test_devourer_usb_device_probe`, and `test_wifi_card_recovery`. Both
regression executables passed. Existing C flag and endian-macro warnings
were emitted during this build; they did not prevent compilation.

The merged driver worktree is clean and whitespace checks passed.
No device deployment, old vendor-SDK rebuild, or RF/range validation is
implied by these checks.
