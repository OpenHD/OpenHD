# MAVLink migration validation

Validation snapshot: 2026-10-13. The source contract and upgrade order are
described in [mavlink-dialect.md](mavlink-dialect.md).

## Code revisions

| Repository | Revision | Result |
| --- | --- | --- |
| OpenHD/mavlink, openhd | 6782304a | XML, generators and compatibility checks passed |
| OpenHD/mavlink-headers, main | 52540ad2 | Regeneration produces an identical checked-in tree |
| OpenHD/OpenHD, openhd-3.0 | 6ec9e328 | Generated statistics and vendor compiler compatibility integrated |
| OpenHD/QOpenHD, dev-release | ae57520c | All eight platform workflows passed |
| OpenHD/openhd-glide, dev-release | 00146c2 | Wire regression passed; corrected Cubie package built and published |
| OpenHD-Technologies/OpenHD-FleetControl, main | 57df9a61 | Build, 93 tests, wire check and production deployment passed |
| OpenHD/OpenHD-WebUI, master | 88f490c3 | Package build and generated wire check passed |

Later OpenHD commits update this report only.

A subsequent OpenHD test-only correction supplies the recording-path hook
required when linking the statistics regression against the common library.
It follows the same test convention as the settings registry tests.

## Build evidence

- [OpenHD package matrix](https://github.com/OpenHD/OpenHD/actions/runs/38306025984):
  AMD64, ARM64, ARMhf, Orqa and X20 succeeded. The complete run is green.
- [OpenHD X21B/Aura](https://github.com/OpenHD/OpenHD/actions/runs/38306025834),
  [Luckfox Lyra](https://github.com/OpenHD/OpenHD/actions/runs/38306025936) and
  [OpenHD build test](https://github.com/OpenHD/OpenHD/actions/runs/38306025853)
  succeeded.
- [OpenHD wire compatibility](https://github.com/OpenHD/OpenHD/actions/runs/38306025662)
  succeeded. Local full OpenHD compilation and the statistics, registry,
  parameter reapply and legacy radio regressions also succeeded after the
  test hook correction.
- [QOpenHD Raspberry Pi](https://github.com/OpenHD/QOpenHD/actions/runs/38304958287)
  and the Rock5, Noble, Jammy, Windows, Fleet and both Android builds succeeded.
  The local Windows build also succeeded.
- [Glide package matrix](https://github.com/OpenHD/openhd-glide/actions/runs/38304633054):
  AMD64, ARM64, ARMhf, Orqa and Orange Pi Zero 3W succeeded. All four Glide
  executables also built locally on Linux.
- [Header regeneration](https://github.com/OpenHD/mavlink-headers/actions/runs/38305846328)
  and [XML generation](https://github.com/OpenHD/mavlink/actions/runs/38305727880)
  succeeded.

## Vendor build follow-up

- [Luckfox Pico rebuild](https://github.com/OpenHD/OpenHD/actions/runs/38312036627)
  succeeded and published its component. Devourer `9e04fca` replaces unsupported
  C++20 initialization/comparison syntax and provides small internal packet-view
  and cooperative-thread fallbacks. Modern libraries retain `std::span` and
  `std::jthread`. Native and fallback tests cover mutable buffer slicing,
  thread moves, replacement cancellation and joining on shutdown.
- Glide Radxa Cubie: the advertised Bullseye security packages return HTTP 404
  from the active mirror but are available from Debian's signed archive. The
  chroot script now uses that archive for Bullseye security sources. The
  [new package matrix](https://github.com/OpenHD/openhd-glide/actions/runs/38312011287)
  passed AMD64, ARM64, ARMhf and Orqa. The archive URL expression has a fixture
  check for both HTTP and HTTPS sources. The corrected
  [targeted Cubie rebuild](https://github.com/OpenHD/openhd-glide/actions/runs/38313855709)
  succeeded and published its package; its workflow also passed actionlint
  1.7.12. Normal builds still
  include both Cedar platforms; the manual `cubie_only` option skips other jobs.
- [Terminal dashboard rebuild](https://github.com/OpenHD/OpenHD/actions/runs/38311512039)
  succeeded after switching to the existing common-library CMake target and
  fetching its dependencies. Local terminal and headless checks also passed.

## Runtime and release boundaries

WebUI and Glide clients were exercised against `192.168.1.124`: both discovered
72 settings and acknowledged a same-value `SYS_DEBUG` write. WebUI additionally
verified parameter metadata over MAVLink FTP and rejection of an invalid
setting value. These checks establish the settings transport, not video or RF
hardware behavior.

FleetControl's main-branch deployment completed. The test Pi now runs OpenHD
`932e7a19` and QOpenHD `ae57520c`, deployed receiver first. This was a binary-only
update; installed package database versions still describe the older packages.
The settings and FTP checks above passed again after deployment, and both
services were active with zero automatic restarts. Process executable hashes
matched the staged CI binaries. Previous binaries are retained under
`/Video/mavlink-wire-20261012/rollback/`. New vendor compiler fixes are not needed
by this Pi's already-built wire receiver. Other devices must also install the
OpenHD receiver before QOpenHD because of the message-1231 CRC correction.
ImageWriter's offline configuration and SysUtils' local interface consume no
changed custom MAVLink messages and required no wire rebuild.

The XML-only upstream review branch is
[upstream/openhd-dialect](https://github.com/OpenHD/mavlink/tree/upstream/openhd-dialect).
It validates against upstream `all.xml`; no upstream PR or allocation approval
is claimed.
