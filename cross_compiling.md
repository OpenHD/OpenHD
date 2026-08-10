# OpenHD portable cross-compilation

The `prepare_cross_sysroots` workflow creates Bullseye sysroots for `arm64` and
`armhf` without QEMU. Each artifact contains a `.tar.zst` archive
and its SHA-256 file. The archives are suitable for static hosting.
The workflow also publishes them as Raw artifacts in the public Cloudsmith
`openhd/dev-release` repository:

```text
https://dl.cloudsmith.io/public/openhd/dev-release/raw/files/openhd-sysroot-bullseye-arm64.tar.zst
https://dl.cloudsmith.io/public/openhd/dev-release/raw/files/openhd-sysroot-bullseye-arm64.tar.zst.sha256
https://dl.cloudsmith.io/public/openhd/dev-release/raw/files/openhd-sysroot-bullseye-armhf.tar.zst
https://dl.cloudsmith.io/public/openhd/dev-release/raw/files/openhd-sysroot-bullseye-armhf.tar.zst.sha256
```

Fetch a hosted sysroot and verify it before extraction:

```bash
./scripts/fetch_cross_sysroot.sh \
  https://example.invalid/openhd-sysroot-bullseye-arm64.tar.zst \
  <sha256-from-the-workflow> \
  /opt/openhd-sysroots/bullseye-arm64
```

Install the matching GNU crosscompiler on the build host and build OpenHD:

```bash
sudo apt-get install gcc-10-aarch64-linux-gnu g++-10-aarch64-linux-gnu cmake pkg-config
OPENHD_SUBMODULE_TOKEN=<token> \
  ./scripts/build_portable_cross.sh \
  arm64 /opt/openhd-sysroots/bullseye-arm64
```

Use `gcc-10-arm-linux-gnueabihf`/`g++-10-arm-linux-gnueabihf` for `armhf`.
AMD64 continues to build natively on the x86 CI runner and needs no sysroot or
emulation.
The cross build forces `ENABLE_LIBCAMERA=OFF`; camera and encoder backends stay
dynamic GStreamer plugins installed by the target image. The Artosyn client,
daemon, and tunnel helper are rebuilt with the same cross toolchain when the SDK
credentials are available.

## Orqa controller

The Orqa controller is not compatible with the portable Debian ARM64 sysroot.
It runs Yocto Scarthgap on AArch64 with glibc 2.39, so use the sysroot and cross
compiler shipped together in the Orqa SDK. After installing the SDK and running
`OpenHD/scripts/setup_orqa_sdk.sh` to add the OpenHD dependencies, build with:

```bash
bash ./scripts/build_orqa_cross.sh /opt/orqa-sdk /tmp/build-openhd-orqa
```

The builder sources `environment-setup-armv8a-poky-linux`, which sets
`SDKTARGETSYSROOT` and the matching compiler tools. It also checks the Poco,
libsodium, and gst-perf runtime ABIs against the controller image before
producing the stripped `openhd` binary.

The controller includes the modern `iw` tool but not the legacy `iwconfig`
command. Build a compatible, self-contained `iwconfig` alongside OpenHD with:

```bash
bash ./scripts/build_orqa_iwconfig.sh /opt/orqa-sdk /tmp/build-openhd-orqa/iwconfig
```
