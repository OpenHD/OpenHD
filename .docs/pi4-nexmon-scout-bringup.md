# Pi 4 internal Wi-Fi scout, 2026-09-06

Scan/Analyse temporarily leases the internal Broadcom radio for Nexmon capture.
The original NetworkManager hotspot, interface name and stock driver return
when the operation finishes. The video adapter continues its existing link
operation; Analyse does not retune it. Hotspot clients disconnect during the
survey because one radio cannot serve its AP channel while sweeping both bands.
Progress remains available through Ethernet or another telemetry transport.

## Implementation

- `scripts/openhd-nexmon-scout` loads the matching experimental module and
  firmware only for a survey. Firmware lookup uses a volatile `/run` override;
  installed stock firmware and modules are untouched. A systemd timer restores
  the original configuration after 600 seconds even if OpenHD exits abruptly.
- The helper validates the running kernel, identifies the internal SDIO radio,
  keeps NetworkManager away during capture, creates `ohdscout`, tunes using
  `nexutil -k<channel>/20`, and checks the frequency independently through `iw`.
  Unsupported frequencies are skipped. `iw set freq` alone returns EBUSY here.
- `NexmonScout` wraps the lease with automatic restoration. Capture parses the
  pinned firmware's 24-byte radiotap format. Stale-channel packets, malformed
  captures, unknown rates and packet loss cannot become false clean evidence.
- `WBLink` routes existing Scan and Analyse commands to this backend when its
  helper and manifest are installed. Backend failure does not fall back to
  sweeping the video adapter.
- Analyse reports foreign packet counts using the existing MAVLink message.
  Three rounds of decoded airtime feed Devourer's `chanmig::RecommendEngine`.
  A short status message reports the best sufficiently observed channel.
  Completion is sent after restoration. Rankings are packet-only advisories;
  missing RF energy measurements and active delivery evidence prevent automatic
  channel migration. Unknown-rate and authenticated own-link traffic invalidate
  a dwell for ranking rather than fabricate external occupancy.
- Scan uses the same authenticated WBTxRx management decoder and keypair as the
  video link. Only authenticated discovery can apply a new primary channel.
  Survey capture is 20 MHz; 10 MHz OpenHD waveforms are unsupported.

## Hardware verification on 192.168.1.124

Raspberry Pi 4 Model B Rev 1.5, ARMv7 kernel `6.1.29-v7l+`, Broadcom BCM43455,
stock firmware 7.45.241. Ethernet supplies management access; `wlan1` normally
serves `openhd_ground`. The USB RTL8812AU uses Devourer for its video link.

- Matching Nexmon firmware and driver loaded without forced symbol versions.
- Real radiotap reception and independent channel readback passed on 2412,
  5180 and 5745 MHz. Capture must use the virtual monitor interface; capture
  on the base managed interface returned no traffic in initial tests.
- Repeated hotspot -> monitor -> hotspot cycles passed, including restoration
  of the original `wlan1` name after the driver initially reappears as `wlan0`.
- Invalid 2413 MHz tune was rejected. Starting the independent systemd restore
  service restored stock firmware and the hotspot without help from OpenHD.
- The C++ live test sampled channels 1/36/149 for three rounds. Devourer ranked
  149 lowest in decoded airtime (0.046% median), then 36 (1.27%), then 1 (8.15%).
  These are observations at that time, not general recommendations.
- Full ARM ground executable built and loaded with all shared libraries found.
  Installed at `/usr/local/bin/openhd`; original retained as
  `/usr/local/bin/openhd.pre-nexmon`. Ground build uses `ENABLE_AIR=OFF` and
  `ENABLE_LIBCAMERA=OFF`; do not distribute this binary as an air build.
- Real MAVLink Analyse command 11201 was accepted and completed, reporting
  traffic on 5700/5745/5785/5825/5260/5280 MHz; unsupported 5865 was skipped.
  Existing primary-radio FHSS continued during the survey.
- The final deployed build repeated Analyse successfully: telemetry reached
  99%, restoration completed, `Wi-Fi best: 5745 MHz (packet-only)` arrived as
  MAVLink status text, then progress reached 100%. The hotspot was back on
  `wlan1`, the lease file was gone, and OpenHD remained active.
- Real MAVLink Scan command 11200 traversed the 2.4 GHz candidates and completed
  with no authenticated air unit found, then restored the hotspot. Positive
  air discovery and live video delivery under a survey still need a working
  paired air unit. No successful discovery is claimed by this test.
- Native parser assertions passed for legacy airtime, channel mismatch,
  unknown rates, malformed/truncated radiotap headers and bad FCS.

## Provisioning and rollback

The tested experimental bundle is included in
`OpenHD/ohd_interface/nexmon/pi4-6.1.29-v7l/`, with checksums, source archive,
notices and rebuild instructions. Both CMake installation and `package.sh`
install it automatically for ARM32 builds. Runtime selection requires a Pi 4
Model B; the helper checks the kernel before touching the radio. No download
or separate driver package is needed. Other boards keep their existing backend.
The helper is installed at `/usr/local/libexec/openhd-nexmon-scout` and these
runtime files at `/usr/local/lib/openhd/nexmon/`:

```
firmware.bin   # bcm43455c0/7_45_206/nexmon
brcmfmac.ko    # brcmfmac_6.1.y-nexmon built for the exact kernel
nexutil       # ARM binary from the same Nexmon checkout
manifest.json # {"kernel":"6.1.29-v7l+", "experimental":true}
```

Source revision: `d6b633800d80b8b8e2132a2c9a5ecda870ec8aaa` from
https://github.com/seemoo-lab/nexmon . Dependencies: Python 3, systemd,
NetworkManager, iw, iproute2, kmod and libpcap. Keep bundle files root-owned.

Native builds can run `test_nexmon_scout`; `test_nexmon_live` requires the Pi,
root and the matching installed bundle. It interrupts the hotspot temporarily.

Manual restoration: `sudo python3 /usr/local/libexec/openhd-nexmon-scout restore`.
To roll back the application, stop `openhd`, restore its `.pre-nexmon` binary,
and start the service. Removing the bundle disables this backend but restores
legacy scanning on the video radio. A reboot clears the temporary firmware
path and loads the installed stock driver. A hard kernel stall may still need
physical recovery; a systemd timer cannot recover a stalled kernel.

The experimental module build used GCC 13.3 to match the kernel's GCC 13
family, while the application uses GCC 10 and a Bullseye ARM sysroot. The
available kernel headers lacked Module.symvers: imported CRCs were recovered
from installed modules via `modprobe --dump-modversions`. The bundled binary
was tested with those CRCs; rebuilding for another kernel requires that kernel
build's complete symbol table. Firmware used Nexmon's embedded GCC 5.4 toolchain.
Local build inputs/outputs are in ignored `out/nexmon-*` and WSL
`/home/damien/openhd-nexmon-build`; the application build is in
`/home/damien/openhd-nexmon-app-armhf`.

## Earlier .42 test and outstanding recovery

192.168.1.42 is a Pi 4 Rev 1.2 with the same kernel. Its initial test successfully
loaded Nexmon and restored the hotspot after an EBUSY tune. A later test stalled
SSH before its banner; recovery has not been confirmed. That earlier method
replaced `/lib/firmware/cypress/cyfmac43455-sdio.bin`, unlike the volatile helper
now used on .124. Restore it from
`/var/lib/openhd/nexmon-backup-20260906/firmware.bin` at the console or after a
power cycle. `scripts/restore_pi_nexmon_test.py` performs that recovery and
reconnects its saved hotspot. Its installed kernel module was never replaced.

Nexmon capture measures decoded Wi-Fi traffic, not arbitrary RF energy.
The upstream Nexmon SDR project documents transmission-only support; this
implementation does not claim a full spectrum analyzer.
