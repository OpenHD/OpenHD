# Pi 4 internal Wi-Fi scout, 2026-09-06

QOpenHD's Find Air Unit screens provide a persisted Normal/Passive switch.
Normal is the default and scans with the primary video radio. Passive explicitly
selects Nexmon and offers 20/40 MHz scan widths. Channel-search command 11200
uses param3 = 0 for Normal (including older clients) and param3 = 1 for Passive;
other values are rejected. Missing Nexmon or unsupported passive widths reject
the request without falling back to the primary radio. Both updated OpenHD and
QOpenHD must be deployed together; older OpenHD versions ignore param3.
Analyse retains its existing automatic scout selection.

Passive Scan/Analyse temporarily leases the internal Broadcom radio for Nexmon capture.
The original NetworkManager hotspot, interface name and stock driver return
when the operation finishes. The video adapter continues its existing link
operation; Analyse does not retune it. Hotspot clients disconnect during the
survey because one radio cannot serve its AP channel while sweeping both bands.
Progress remains available through Ethernet or another telemetry transport.

## Extended-channel experiment, 2026-09-16

An isolated driver/firmware probe build and test procedure are in
`OpenHD/ohd_interface/nexmon/extended/README.md`. The driver adds entries
5845/5865/5885/5905/5925 MHz. A firmware diagnostic ioctl attempts the original
setter and reports its result, PHY lookup and current chanspec. Firmware
validation is retained; actual reception on these channels is not verified.
The bundle is selected explicitly for a test lease and is not automatically
packaged or selected for ordinary Scan/Analyse.

The driver and firmware compiled with the documented GCC 13.3/5.4 toolchains.
The new module's 260 imported symbol CRCs match the tested baseline and its
vermagic matches 6.1.29-v7l+. Bundle checksums and source inclusion were checked.
Nine protocol assertions verify that rejected tunes, stale/wrong readbacks,
missing PHY channels and unpatched firmware cannot authorize packet capture.
Live baseline, extended-channel off/on captures and restoration remain to test
on the Pi with a known transmitter and an SDR covering the test frequencies.

## Devourer/Nexmon reception check, 2026-09-16

`test_nexmon_devourer` is a bounded hardware tool that sends 20 MHz OpenHD
management announcements with the normal default keypair through Devourer, or
receives them through the authenticated WBTxRx decoder on an already leased
`ohdscout`. It does not own/restore the scout lease; its caller must do so.

Two completed checks on 5745 MHz did not detect the test announcements:
first .42's RTL8822EU transmitting to .124's Nexmon, then .124's RTL8812AU
transmitting to its own internal Nexmon radio. Each sender completed 300
injection calls; each ten-second receiver reported zero packets and zero
authenticated announcements. The same-Pi run additionally recorded zero raw
frames in AF_PACKET capture (a 24-byte, header-only PCAP). These results do
not establish over-the-air TX: there was no independent RF witness.
Both completed leases restored successfully, and OpenHD was restarted after
the same-Pi test.

During lease setup for a subsequent 2412 MHz control, .124 stopped responding
to SSH, preventing capture and confirmation of recovery. Further RF testing
was stopped. Independent scout restoration and application restart timers had
been scheduled; they cannot recover a stalled kernel. This run used the
existing bundled Nexmon firmware/module, not the extended-channel build.
The next step is to recover .124, inspect its kernel logs, and establish a
known-good baseline RX/TX before interpreting extended-channel tests.

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
  A 10 MHz-only request is rejected; mixed requests skip 10 MHz and continue
  discovery at 20 MHz. When likely OpenHD packets arrive, Scan allows up to
  five extra seconds for the session key and authenticated channel announcement,
  as the primary-radio scan does. Per-channel debug logs include total, likely
  OpenHD and authenticated packet counts and the announced frequency/width.

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
