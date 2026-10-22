# Building OpenHD with make

The top-level `Makefile` is a convenience entry point. It does not replace the
CMake project in `OpenHD/`: `cmake -S OpenHD -B OpenHD/build` keeps working
with the `option()` defaults, and Buildroot, Yocto and the CI scripts use that
path unchanged.

## Native builds

```bash
sudo ./install_build_dep.sh ubuntu-x86
make setup          # .venv with kconfiglib (uses uv when installed)
make air            # full binary (air and ground at runtime)
make ground         # ground-only binary without the camera stack
make menuconfig     # adjust .config, then: make
make test           # ohd_common unit tests
```

`make` writes `.config` (Kconfig) and turns it into
`OpenHD/build/openhd-config.cmake`, which is passed to CMake with `-C`. Kconfig
symbols carry the CMake option names (`ENABLE_AIR`, `ENABLE_USB_CAMERAS`,
`ENABLE_GSTREAMER`, …), so a new CMake option only needs a matching entry in
`Kconfig` to become configurable.

Enabling *Resolve and build Artosyn SDK support* runs
`OpenHD/scripts/resolve_artosyn_sdk.sh` first, like `OpenHD/build_cmake.sh`.

Useful variables: `BUILD_DIR`, `NPROC`, `GENERATOR`, `CMAKE_ARGS`
(for example `make CMAKE_ARGS=-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`).

## Cross and platform builds

These targets call the existing scripts and ignore `.config`:

| Target | Script |
| --- | --- |
| `make orqa SDK=<sdk> [OUT=<dir>]` | `scripts/build_orqa_cross.sh` |
| `make portable ARCH=arm64\|armhf SYSROOT=<dir> [OUT=<dir>]` | `scripts/build_portable_cross.sh` (see `cross_compiling.md`) |
| `make rk3588` | `OpenHD/build_rk3588_cross.sh` |
| `make x21 SDK=<sdk> OUT=<dir>` | `scripts/build_x21_component.sh` |
| `make luckfox SDK=<sdk> OUT=<dir>` | `scripts/build_luckfox_component.sh` |
| `make lyra SDK=<sdk> OUT=<dir>` | `scripts/build_lyra_ground_component.sh` |
