# OpenHD portable cross-compilation

The `prepare_cross_sysroots` workflow creates Bullseye sysroots for `arm64` and
`armhf` without QEMU. Each artifact contains a `.tar.zst` archive
and its SHA-256 file. The archives are suitable for static hosting.

Fetch a hosted sysroot and verify it before extraction:

```bash
./scripts/fetch_cross_sysroot.sh \
  https://example.invalid/openhd-sysroot-bullseye-arm64.tar.zst \
  <sha256-from-the-workflow> \
  /opt/openhd-sysroots/bullseye-arm64
```

Install the matching GNU crosscompiler on the build host and build OpenHD:

```bash
sudo apt-get install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu cmake pkg-config
OPENHD_SUBMODULE_TOKEN=<token> \
  ./scripts/build_portable_cross.sh \
  arm64 /opt/openhd-sysroots/bullseye-arm64
```

Use `gcc-arm-linux-gnueabihf`/`g++-arm-linux-gnueabihf` for `armhf`.
AMD64 continues to build natively on the x86 CI runner and needs no sysroot or
emulation.
The cross build forces `ENABLE_LIBCAMERA=OFF`; camera and encoder backends stay
dynamic GStreamer plugins installed by the target image. The Artosyn client,
daemon, and tunnel helper are rebuilt with the same cross toolchain when the SDK
credentials are available.
