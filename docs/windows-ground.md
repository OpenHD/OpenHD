# Windows ground dashboard

Extract the portable ZIP and double-click **openhd_ground.exe**. No command-line
arguments are needed. This is the native Devourer ground backend with the same
shared terminal dashboard renderer used by the Linux OpenHD executable.
It links the repository-pinned `OpenHD/wifibroadcast` fork from
`OpenHD/ohd_interface/lib/wifibroadcast`. Devourer supplies USB radio access;
OpenHD's `WBTxRx`, `WBStreamRx`/`WBStreamTx`, encryption, FEC and radio ports
provide compatibility with the Linux air unit.

On the first launch, the settings screen opens. Choose the USB adapter (or leave
**Automatic discovery**), frequency, channel width and optional forwarding
settings, then press **S** to save and start. Defaults use the same built-in keypair
as Linux OpenHD; a custom 128-byte keypair file is optional.

Settings are stored in `%APPDATA%\OpenHD\ground.json`. Later launches restore them
and start the backend when **Start automatically** is enabled. If the adapter is
missing or cannot be opened, the dashboard stays open and shows the error in its
logs. Connect/fix the adapter and press **R**, then **Y**, to retry.

## Dashboard controls

| Key | Action |
| --- | --- |
| C | Ground settings: radio, channel, keys, UDP destinations and ports |
| R, then Y | Start or restart the ground backend |
| D | Debug-source filters, persistent logging and log export |
| N | Network addresses |
| Tab | Dashboard / full-screen logs |
| Up/Down, PgUp/PgDn, End | Scroll logs / return to live logs |
| P, then Y | Stop the backend; Windows keeps running |
| Q or Ctrl+C | Exit and close USB/receiver threads |

In Settings, use Up/Down and Enter. Type to replace a field, Backspace to edit,
Ctrl+U to clear, S to save/start, and Esc to cancel. Frequency/width must match the
air unit. The Windows adapter's Wi-Fi USB interface must use a libusb-compatible
driver such as WinUSB. It is then driven by Devourer rather than Windows Wi-Fi.

The dashboard shows the same OpenHD logo, resource meters, link/device/network
panels, live logs and debug menus as Linux. Windows CPU/RAM/disk/network values
come from Windows APIs; unavailable measurements such as CPU temperature remain
unknown. A configured radio does not prove reception from the air unit.

Log exports and optional persistent logs are stored beside the profile in
`%APPDATA%\OpenHD\logs`. Persistent logs rotate at 10 MiB, retaining one previous
file. QOpenHD remains the video/OSD application; this executable is the ground
backend and terminal dashboard.

## Streams and scope

Primary/secondary video are forwarded unchanged to UDP 5600/5601. Telemetry goes
to UDP 14550. Localhost UDP 14551 accepts telemetry uplink datagrams up to 1024
bytes. All endpoints can be changed in Settings. The backend uses the existing
OpenHD decryption, multiplexing and FEC pipeline in the same process as the UI.

This initial Windows target does not include the Linux system services, MAVLink
parameter/settings server, channel scanning, FHSS, camera support or plugin
crypto. The local Settings menu configures the Windows ground backend; it does
not change air-unit settings remotely. Actual RF, video and telemetry operation
still needs a compatible adapter and a matching air unit.

## Build

Libraries (libusb, libsodium, PDCursesMod, spdlog, nlohmann-json) come from
[vcpkg](https://vcpkg.io) in manifest mode (`OpenHD/vcpkg.json`, versions pinned by
the baseline in `OpenHD/vcpkg-configuration.json`). You need:

- vcpkg with `VCPKG_ROOT` set (e.g. `scoop install vcpkg`, or a clone of
  microsoft/vcpkg plus `bootstrap-vcpkg.bat`),
- a MinGW-w64 GCC 13+ (UCRT), CMake 3.21+ and Ninja on `PATH` — MSYS2 UCRT64
  (`pacman -S mingw-w64-ucrt-x86_64-{gcc,cmake,ninja,pkgconf}`) or scoop
  (`mingw`, `cmake`, `ninja`, `pkg-config`).

```sh
git submodule update --init OpenHD/ohd_interface/lib/wifibroadcast OpenHD/ohd_interface/lib/devourer
cd OpenHD
cmake --preset windows-ground
cmake --build --preset windows-ground
ctest --preset windows-ground
../out/windows-ground/platform/ground/openhd_ground.exe
```

The first configure builds the dependencies from source (~15 min, mostly
libsodium and libusb); vcpkg's binary cache (`%LOCALAPPDATA%\vcpkg\archives`)
makes every later configure, in any build directory, take well under a minute.
Changing the GCC version invalidates that cache. The preset uses the overlay
triplet `x64-mingw-static-release` and links with `-static`, so
`openhd_ground.exe` needs no DLLs beyond those shipped with Windows 10+.

MSVC is not supported by this target yet. Without vcpkg the CMake files still
fall back to pkg-config/system packages and the in-tree spdlog/json submodules.
Normal Linux builds keep their existing entry point and startup behavior.

## Optional headless operation

The original CLI remains available for scripts and diagnostics:

```powershell
.\openhd_ground.exe --list
.\openhd_ground.exe --no-ui --usb 0bda:8812 --frequency 5180 --width 20
```

Use `--help` for overrides, `--config FILE` for another saved profile and
`--configure` to open Settings immediately. A custom key is passed with
`--keypair FILE`; otherwise the Linux-compatible default is derived. Explicit
`--device devourer-usb-BUS-ADDR` addresses are available for advanced setups;
Windows bus numbering can change between libusb contexts, so prefer automatic
selection or stable VID/PID selection for a single adapter. Ambiguous identical
adapters are refused without resetting either one.
