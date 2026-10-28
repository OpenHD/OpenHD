# Integrated OpenHD GUI

Enable with `-DOPENHD_ENABLE_GUI=ON` when configuring OpenHD. The default is
`OFF`, so normal builds do not need Qt. The GUI currently only displays
**Hello World**.

This is part of the existing `openhd` (Linux) / `openhd_ground.exe` (Windows)
process. `OpenHD/gui` is a static library, not a separate executable or service.
Its `openhd::ui::Gui` object is owned on the main thread, alongside the core
objects. Add controls here and pass core objects or callbacks directly from
`main.cpp`; update widgets only from the main thread. Backend workers continue
running while the main loop services Qt events every 16 ms. Closing the window
uses the core's normal cleanup path.

## Raspberry Pi / Linux

Install Qt Widgets development files (`qtbase5-dev` on Debian), then use the
usual OpenHD dependencies and build:

```sh
cmake -S OpenHD -B out/gui -DOPENHD_ENABLE_GUI=ON
cmake --build out/gui --target openhd --parallel 2
QT_QPA_PLATFORM=eglfs ./out/gui/openhd -g
```

Linux defaults to EGLFS, which opens fullscreen. The installed Qt must include the EGLFS platform plugin
and a device integration matching the Pi's EGL/GLES/KMS graphics stack.
Touch uses Qt's built-in libinput/evdev handling. The runtime user needs access
to the display/GPU and `/dev/input/event*` devices. No extra touch driver is
implemented in OpenHD. See Qt's [embedded Linux guide](https://doc.qt.io/qt-6/embedded-linux.html)
and [input guide](https://doc.qt.io/qt-6/inputs-linux-device.html).

Use `QT_QPA_PLATFORM=xcb` for a Linux desktop or `offscreen` for headless checks.
Select the platform through the environment, since OpenHD retains its own
command-line parser. A GUI-enabled Linux build replaces the curses dashboard.
The normal core requirements (root, SysUtils, configuration and hardware) still
apply. Ensure no other application/compositor owns the display when using EGLFS.

For cross builds, create a Qt-enabled sysroot using the existing builder:

```sh
sudo env OPENHD_ENABLE_GUI=ON bash scripts/create_cross_sysroot.sh armhf /tmp/openhd-gui-sysroot bullseye
OPENHD_ENABLE_GUI=ON bash scripts/build_portable_cross.sh armhf /tmp/openhd-gui-sysroot /tmp/openhd-gui-build
```

Both the sysroot and OpenHD configuration need the flag. The armhf CI builds
the Linux core against Bullseye Qt 5; the artifact requires matching Qt runtime
libraries/plugins on the target. This is an ARMv7 hard-float build for Pi 2/3/4,
not the ARMv6 Pi 1 / original Zero.

## Windows

Use the existing [MinGW UCRT64 ground build](windows-ground.md) with the extra
MSYS2 package `mingw-w64-ucrt-x86_64-qt5-base` and CMake argument
`-DOPENHD_ENABLE_GUI=ON`. Run `openhd_ground.exe` from the generated package.
Qt uses its native Windows platform plugin. The GUI runs alongside the normal
ground backend (when saved settings enable autostart); it remains open if no
radio is connected. `--no-ui` keeps headless operation, and `--configure` opens
the existing console settings editor. There are no graphical settings yet.

The **OpenHD integrated GUI** workflow builds both core executables and uploads
`openhd-gui-windows` (including Qt runtime) and `openhd-gui-pi-armhf` artifacts.
The Linux job uses the same private-submodule token as the normal core CI and
is skipped for fork pull requests. Build success does not verify a physical
Pi display or touchscreen.
