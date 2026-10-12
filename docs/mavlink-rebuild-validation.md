# MAVLink migration validation

Validation snapshot: 2026-10-12. The source contract and upgrade order are
described in [mavlink-dialect.md](mavlink-dialect.md).

## Published revisions

| Repository | Revision | Result |
| --- | --- | --- |
| OpenHD/mavlink, openhd | 6782304a | XML, generators and compatibility checks passed |
| OpenHD/mavlink-headers, main | 52540ad2 | Regeneration produces an identical checked-in tree |
| OpenHD/OpenHD, openhd-3.0 | 932e7a19 | Generated statistics integrated; see platform results below |
| OpenHD/QOpenHD, dev-release | ae57520c | All eight platform workflows passed |
| OpenHD/openhd-glide, dev-release | 671775b0 | Native build and wire regression passed |
| OpenHD-Technologies/OpenHD-FleetControl, main | 57df9a61 | Build, 93 tests, wire check and production deployment passed |
| OpenHD/OpenHD-WebUI, master | 88f490c3 | Package build and generated wire check passed |

A subsequent OpenHD test-only correction supplies the recording-path hook
required when linking the statistics regression against the common library.
It follows the same test convention as the settings registry tests.

## Build evidence

- [OpenHD package matrix](https://github.com/OpenHD/OpenHD/actions/runs/38306025984):
  AMD64, ARM64, ARMhf and Orqa succeeded. X20 status must be read from the run.
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

## Remaining platform build blockers

- [Luckfox Pico](https://github.com/OpenHD/OpenHD/actions/runs/38306025895):
  the vendor GCC 8.3 C++ library lacks `<span>`, required by Devourer's
  `jaguar1/FrameParser.h`. The backend is now correctly checked out; completing
  this target requires a compatible SDK or a separately validated backend
  portability change.
- Glide Radxa Cubie: Debian's advertised
  `libgbm-dev_20.3.5-1+deb11u1_arm64.deb` returns HTTP 404. This occurs before
  compilation. No downgrade of the graphics runtime was made.
- [Terminal dashboard workflow](https://github.com/OpenHD/OpenHD/actions/runs/38306026382):
  its standalone command does not fetch/include the spdlog dependency. This
  separate workflow is not evidence of a MAVLink package failure.

## Runtime and release boundaries

WebUI and Glide clients were exercised against `192.168.1.124`: both discovered
72 settings and acknowledged a same-value `SYS_DEBUG` write. WebUI additionally
verified parameter metadata over MAVLink FTP and rejection of an invalid
setting value. These checks establish the settings transport, not video or RF
hardware behavior.

FleetControl's main-branch deployment completed. The new OpenHD/QOpenHD wire
builds have not been installed on the test device. Install the new OpenHD
receiver before QOpenHD because of the historical message-1231 CRC correction.
ImageWriter's offline configuration and SysUtils' local interface consume no
changed custom MAVLink messages and required no wire rebuild.

The XML-only upstream review branch is
[upstream/openhd-dialect](https://github.com/OpenHD/mavlink/tree/upstream/openhd-dialect).
It validates against upstream `all.xml`; no upstream PR or allocation approval
is claimed.
