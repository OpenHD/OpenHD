# OpenHD WiFi Test

Build with `-DOPENHD_ENABLE_GUI=ON` to enable the in-process RF laboratory
broadcast tool. It runs inside `openhd` on Linux and `WifiTest.exe` on
Windows. The flag defaults to OFF: regular OpenHD builds do not find or link
Qt or the laboratory code. Windows GUI builds produce `WifiTest.exe` with the OpenHD icon and do not create a terminal.

The GUI controller owns the selected USB radio exclusively. Normal receiver,
telemetry and video workers are not started in GUI mode. Nothing transmits on
launch. Select a detected adapter and settings, then press **Start**. **Apply
settings** ends the previous segment and applies the current dropdown values;
**Stop** ends transmission. There is no sweep, scheduler or automatic tuning.
The timeline shows actual applied segments and gaps, with requests/errors in
the event table. Closing the window joins the worker and closes the radio.

## Controls and backend

Adapter discovery uses the shared read-only Devourer silicon probe. Only
recognized chipsets and allowlisted Windows adapters awaiting driver setup enter
the dropdown; unrelated USB devices are excluded. Transmission requires a
verified, accessible chipset. Jaguar adapters need
WinUSB/USB read access to verify their chipset. Refresh runs off the UI thread.
Known retail identities include `ASUS USB-AC56 | RTL8812AU` for `0b05:17d2`
with confirmed 8812A silicon. Generic/shared USB IDs retain chipset names
without guessing a retail model. IDs and bus/address remain visible for
distinguishing multiple adapters; the tooltip includes the full identity.

All channels come directly from OpenHD's `wifi_channel.h`, including extended
channels. Widths (currently 10/20/40 MHz), HT MCS (0..31), and STBC (0..3) come
from shared OpenHD validators. The visible controls are frequency, width, MCS,
power, STBC, LDPC, short GI, and payload. Payload tuning is automatic. Hardware
capabilities can reject settings such as an MCS requiring too many spatial
streams, a band the adapter cannot tune, or unsupported LDPC/STBC/GI. Such a
request is recorded as an error and transmission stops. An unavailable control
is never recorded as successfully applied.

Power uses OpenHD's normalized 0..150% levels, including its adapter-relative
overdrive range, and the same calibrated `devourer_power_offset_qdb()` /
`set_devourer_tx_power_level()` implementation. Percentages are not dBm.
The actual quantized offset and available register diagnostics are recorded.
5 MHz remains disabled by OpenHD's validator; 80 MHz
is not supported by that transmit header.

The tool uses `WBTxRx`, `WBStreamTx`, the standard air identifier and primary
video radio port, with OpenHD's default bind key and authenticated unencrypted
payloads. The same Devourer USB transport opens, tunes and powers the radio.
It does not substitute a socket-only or simulated transmit path. No receiver
is required: the other end may be an RF laboratory instrument.

- **Garbage** is deterministic MT19937 data with a fixed seed of 42. It fills
  the full normal OpenHD variable-bitrate payload budget for the selected
  frequency, width and MCS. The same production rate tables and FEC accounting
  are shared, with the normal 100% budget and default 20% FEC. The offered rate
  is recalculated whenever settings are applied; no manual bitrate cap is used.
- **Sample video** loops the bundled 30-frame 320x180 H.264 pattern through
  OpenHD's librtp packetizer at that same automatically calculated payload rate.
  Sequence numbers remain continuous and timestamps use a 90 kHz clock.
- Packets use a fixed 1434-byte limit. FEC uses OpenHD's encoder with an
  automatic 20-primary-fragment block limit and normal no-ACK transmit flag.
  There are no beamforming, FEC/block, rate, MTU, seed, or TX-flag controls.
- Host pacing follows the normal link budget and never emits unlimited catch-up
  bursts. Queue rejections, late pacing and TX failures remain recorded; they
  do not silently reduce the configured VBR budget.

## Sample data

Each Start creates a unique directory under the application's local-data
`wifi-tests` folder. The directory is displayed in the GUI. After Stop,
**Export session** copies it to a selected folder. Files are flushed during a run:

- `session.jsonl`: schema version, source revision, sample-video SHA-256,
  requested/applied settings, capability report, segment start/end and errors,
  UTC microseconds plus monotonic elapsed microseconds, and measurements.
- `samples.csv`: a 250 ms series of settings, quantized power offset, generated
  and queued packets, pacing overruns, cumulative WBTxRx injection counters,
  drop/error counters, and raw thermal values when available. Injection counters
  are cumulative for the session; offered-source counters reset each segment.
- `diagnostics.log`: the available stdout/stderr diagnostics from OpenHD,
  wifibroadcast and Devourer, with ANSI display formatting removed.

The app records **backend acceptance**, not proof of physical modulation or RF
power. dBm and receiver loss fields are null because only the external lab can
measure them in this setup. Host pacing is not a hardware-timed waveform.
Firmware/session-key traffic and FEC overhead contribute additional airtime.
Periodic register diagnostics also consume USB control bandwidth. Preserve the
hardware/antenna/lab setup alongside the exported data when comparing tests.

## Build and runtime

Linux needs Qt Widgets (`qtbase5-dev`) in addition to normal OpenHD dependencies:

```sh
cmake -S OpenHD -B out/gui -DOPENHD_ENABLE_GUI=ON
cmake --build out/gui --target openhd --parallel 2
QT_QPA_PLATFORM=eglfs ./out/gui/openhd
```

EGLFS is the Linux default and opens fullscreen. Qt must have an EGLFS device
integration matching the GPU/KMS stack and access to `/dev/input/event*` for
its built-in libinput/evdev touch handling. Use `QT_QPA_PLATFORM=xcb` on a Linux
desktop or `offscreen` for automated checks. The USB radio needs suitable
permissions and must not be owned by another OpenHD process.

For armhf cross builds, create a Qt-enabled sysroot with
`OPENHD_ENABLE_GUI=ON scripts/create_cross_sysroot.sh`, then pass the same flag
to `scripts/build_portable_cross.sh`. CI builds ARMv7 hard-float for Pi 2/3/4;
Pi 1 and the original Zero need ARMv6 builds. Target images need matching Qt
runtime libraries and EGLFS plugins; the artifact is not a complete Pi image.

Windows uses the existing MinGW UCRT64 build with Qt 5 base/tools and ANGLE.
The CI package includes Qt and its dependencies. Install WinUSB for the chosen
adapter. The regular console backend remains available with GUI disabled;
`--no-ui` on a Windows GUI build keeps its existing headless receiver behavior.

CI builds Pi armhf and Windows, checks the Windows GUI subsystem, runs payload
and UI contract tests, and smoke-tests the packaged Windows application. These
checks do not establish physical RF or touchscreen correctness.

## Realtek manufacturer mode investigation

The locally supplied rtwpriv 5.9.1 source is a Linux private-ioctl frontend:
`rtw_api.cpp::wlan_ioctl_mp()` sends `RTW_IOCTL_MP` to a kernel driver. Its command
parser alone cannot implement manufacturer tests on Windows. A Windows vendor
driver with a documented MP interface, or device-specific Devourer routines,
is required. Existing Devourer chip classes contain some CW/continuous-TX
routines; support needs per-chip inspection and hardware validation. This GUI
currently implements OpenHD packet broadcast only. It does not enable MP mode,
write EFUSE, or claim vendor manufacturing-test parity.

### Windows USB driver setup

The Windows GUI uses libwdi (the open-source library behind Zadig) directly.
No Zadig application or manual device selection in another tool is needed.
An allowlisted adapter on its vendor driver appears as **WinUSB setup required**,
with its USB ID and current driver in the details. ASUS `0b05:17d2` is identified
as **ASUS USB-AC56 / RTL8812AU** before the silicon probe becomes accessible.

Select the adapter and use **Set up WinUSB**. Confirm the displayed adapter and
Windows administrator prompt. The same `WifiTest.exe` runs its integrated
installer, then refreshes automatically. Start remains disabled until the
read-only Devourer probe identifies a supported, accessible chip. Replacing the
vendor driver removes normal Windows Wi-Fi access on that adapter; Device
Manager can restore the vendor driver. Windows may bind identical adapters to
the same driver package.

libwdi and the Microsoft WinUSB redistributables are prepared only by the optional
Windows GUI CI. Regular OpenHD builds do not find, fetch, link, or package them.
The Windows package includes the libwdi DLL, its license, and the exact source
archive used to build it. GUI Windows developers supply `OPENHD_LIBWDI_ROOT`
pointing to that library's installed prefix.
