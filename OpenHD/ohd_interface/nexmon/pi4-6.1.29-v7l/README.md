# Bundled Pi 4 Nexmon scout

This directory contains the exact module, patched firmware and static ARM32
nexutil tested on OpenHD's 6.1.29-v7l+ Pi 4 image. SHA256SUMS covers every supplied
file other than itself. CMake verifies it before packaging. No downloads or
compilation are needed on the installed image.

Both the CMake install and package.sh place these files under
/usr/local/lib/openhd/nexmon and the helper under /usr/local/libexec. Only
32-bit ARM builds install this bundle; runtime selection requires a Pi 4 Model B.
The helper also checks uname -r before changing anything. This is not a module
for Pi 5, ARM64 kernels or other kernel builds, even if an image has a similar name.

OpenHD loads it only when Scan/Analyse is requested. The stock module and firmware
are not overwritten, and the original hotspot is restored after the survey.
The independent restore timer handles application failure, not a stalled kernel.
The C++ integration and operational test results are documented in
.docs/pi4-nexmon-scout-bringup.md in the OpenHD repository.

## Source and rebuild

Upstream: https://github.com/seemoo-lab/nexmon
Commit: d6b633800d80b8b8e2132a2c9a5ecda870ec8aaa

nexmon-source.tar.gz is a git archive of the driver, utility, firmware patch,
base firmware and shared source/build files from that commit. Sources have not
been modified. LICENSE.nexmon and the original per-file notices apply to the
third-party materials; they are not relicensed by OpenHD. The driver retains
its upstream ISC/BSD/GPL notices; the Nexmon patches and utility carry their
upstream GPL notices. Keep the supplied notices with redistributed files.

Extract the source archive into a working directory. Full upstream checkout is
also usable. Export NEXMON_ROOT to that absolute directory. Kernel headers must
come from the exact OpenHD kernel build, with CONFIG_MODVERSIONS and its matching
Module.symvers. The supplied kernel-Module.symvers.gz (decompress with gzip -d) records the imported symbol
CRCs recovered from that image for the tested binary. It is a bring-up artifact,
not a substitute for the complete symbol table when building a different kernel.
No force-modversion or permanent driver replacement is required.

Driver build on a Linux host, with the exact kernel headers and ARM GNU 13.3
compiler (the image kernel was built with GCC 13):

```sh
export NEXMON_ROOT=/absolute/path/to/extracted-source
export CROSS_COMPILE=/absolute/path/to/arm-none-linux-gnueabihf-
make -C "$KERNEL_BUILD" ARCH=arm CROSS_COMPILE="$CROSS_COMPILE" \
  M="$NEXMON_ROOT/patches/driver/brcmfmac_6.1.y-nexmon" -j4
```

The original headers' host modpost needed rebuilding for the build host's libc;
use the kernel's own modpost sources if that host-tool incompatibility occurs.
Use the resulting patches/driver/brcmfmac_6.1.y-nexmon/brcmfmac.ko. The module's
brcmutil and cfg80211 dependencies are explicitly loaded by the runtime helper.

Firmware build uses the upstream x86 ARM embedded GCC 5.4 toolchain and matching
Nexmon GCC plugin. The toolchain is available in the pinned upstream repository
under buildtools/gcc-arm-none-eabi-5_4-2016q2-linux-x86; it is not needed at runtime.
Follow upstream's host prerequisites (including 32-bit host libraries and
zlib-flate), then from NEXMON_ROOT:

```sh
touch DISABLE_STATISTICS
source setup_env.sh
make -C buildtools/flash_patch_extractor
make -C firmwares/bcm43455c0/7_45_206
make -C patches/bcm43455c0/7_45_206/nexmon \
  brcmfmac43455-sdio.bin GIT_VERSION=d6b6338
```

The tested firmware identifies itself as 7.45.206 (nexmon.org: d6b6338-3).
The build counter and toolchain affect byte-for-byte reproduction. Rename the
resulting patched brcmfmac43455-sdio.bin to firmware.bin. Do not use the base
firmware from the firmwares/ directory as the patched result.

Build nexutil on a Bullseye ARM32 Pi using the source archive:

```sh
make -C utilities/nexutil
```

This produces the static nexutil executable using the legacy netlink API used
by the helper. Firmware tune acknowledgements are not trusted alone: the helper
checks the actual frequency through iw after each tune.

After replacing any artifact, regenerate SHA256SUMS and repeat the parser,
real capture, command progress, and hotspot restoration tests before shipping.
