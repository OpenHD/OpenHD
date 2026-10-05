# Air/ground connection investigation, 2026-10-05

Live checks used Air 192.168.1.42 and Ground 192.168.1.124. The current binaries
and service units were retained, and unrelated worktree changes were preserved.
The sections below record successive diagnostic stages; the final recovery section
supersedes the earlier disconnected state. Runtime radio changes and temporary
kernel-driver isolation are documented there. No camera settings were explicitly
changed, but OpenHD's automatic bitrate control changed the encoder target.

## Initial blocker: Air USB radio is absent

Air `lsusb` lists only the Raspberry Pi root hubs and VIA hub. The 8812EU adapter
does not enumerate. Its boot kernel log contains the hub enumeration but no
Realtek adapter enumeration. This is below OpenHD's device discovery layer; these
checks cannot distinguish a cable, connector, adapter, or external-supply problem.
Both Pis currently report `get_throttled=0x0`; that does not measure a separate
adapter supply.

Air's persistent log records `No openhd wifibroadcast card found`, `Link not
functional`, and `monitor_mode card(s):size:0{}`. It starts Ethernet instead.
There is no Air wifibroadcast settings file. Component 191 exposes 41 parameters,
none of which is `WB_FREQUENCY`, `WB_CHANNEL_W`, or `WB_N_CARDS`.

The Air log also captures the user's earlier bandwidth requests for 10 and 20 MHz
failing with `WB_CHANNEL_W does not exist` and `MISSING_PARAM`. This explains those
requests without assuming a retuning failure in a detected Air radio.

The IMX708 is probed by the kernel. The running rpicam process encodes 1280x720 at
60 fps; telemetry reports roughly 8 Mbps encoded output. No decoded display or
RF camera delivery was verified. Sysutils calls this camera ARDUCAM_SKYMASTERHDR.

## Ground tuning works independently

Ground USB identity is 0bda:b812. OpenHD identifies RTL8822B/Jaguar2 and starts its
Devourer transport. Component 191 exposes 67 parameters, including the RF settings.

Live MAVLink tests successfully changed Ground through these sequences:

- 5745 -> 5785 -> 5745 MHz.
- 5745 -> 2412 -> 5745 MHz.

Each request received an accepted PARAM_EXT_ACK. Follow-up parameter reads and
radio/application logs confirmed application and persistence. The final telemetry
reports 5745 MHz at 20 MHz width. The earlier user-triggered channel scan also
visited multiple channels and returned to 5745 MHz.

Ground received zero RF packets and zero RF video blocks during the samples.
Its injection-error hints rose in one sample; reception/transmission quality and
paired channel following remain untested while Air's adapter is absent.

## Additional code findings

1. **Late first attachment has no recovery path.** Discovery times out after about
   10 seconds in `OpenHD/ohd_interface/src/wifi_card_discovery.cpp`.
   `OHDInterface` creates WBLink and starts its recovery supervisor only when
   startup already found a monitor-mode card (`ohd_interface.cpp:153-168`).
   An adapter first attached after this scan requires restarting OpenHD. The
   existing supervisor handles replug of an already-created WBLink.

2. **UI liveness does not establish RF capability.**
   `QOpenHD/qml/ui/configpopup/openhd_settings/LinkQuickPanel.qml:188-205` chooses
   synchronized versus single-unit frequency changes from system heartbeats.
   Air reached over TCP/Ethernet can be alive while exposing no WB parameters.
   The bandwidth and MCS models in `WifiBroadcastLinkCard.qml:78-90` use Air
   metadata exclusively, so a Ground-only connection has no usable Air metadata.
   These are source findings; this investigation did not exercise the rendered UI.

3. **Ethernet discovery is not automatic failover.** Both units log discovery of
   each other over Ethernet. Ground's active route is WIFIBROADCAST; Air's is
   Ethernet. `OHDInterface::apply_link_policy()` chooses an existing WBLink before
   Ethernet without checking whether an Air RF peer is alive. Ground has
   MULTI_LINK_CAP=0 and MULTI_LINK_EN=0. Setting WFB_LINK_EN=0 was rejected with
   PARAM_EXT_ACK result 1: the callback explicitly forbids that change outside
   enterprise multi-link mode (`ohd_interface.cpp:712`). The setting remains 1.
   No Ethernet fallback delivery was established.

## Versions and evidence

Both devices run package `3.0-alpha-202610041803-9d998bcc`, kernel 6.1.29-v7l+,
and identical OpenHD binary SHA256:
`32f1657499500b9aa4043c1e25467e055817d93c3040a7923f31d54ed8bef35e`.

Raw evidence is in ignored local `out/`: initial/details device snapshots,
`connection-param-inventory.json`, and telemetry captures
`wfb-telemetry-1791203505.json`, `wfb-telemetry-1791203606.json`, and
`wfb-telemetry-1791203626.json`. Device persistent logs are under
`/Video/logs/openhd/`. Air boot timestamps initially read 2023 until its clock
updated; use boot ordering rather than those timestamps for elapsed-time claims.
Private diagnostic helpers contain credentials and must stay local.

## Plan recorded before USB recovery

Reconnect/check the Air adapter and its separate supply until it appears in
`lsusb`. Restart Air OpenHD after enumeration because of the late-attachment gap.
Then re-fetch parameters, confirm Air and Ground frequency/width, and validate RF
telemetry and video independently of Ethernet before testing synchronized channel
changes. Ground was left at its original 5745 MHz, width20, PIT enabled, plain
coding, diagnostic UDP disabled, WFB_LINK_EN=1. No claim of restored RF connection
or paired channel-change success is made.

## Follow-up: adapter briefly enumerated, then USB failed

On the next user-requested run, Air briefly exposed USB device 19, 0bda:a81a.
It then disappeared before OpenHD could initialize it. The kernel log records
successful kernel-driver firmware loading followed by a USB disconnect; a later
firmware download fails after another disconnect. Port 1-1.2 then attempts
full-speed enumeration, fails repeated resets with error -71, and reports
`unable to enumerate USB device`. The upstream VIA hub also reports
`hub_ext_port_status failed (err = -71)`.

Air OpenHD was restarted once to rediscover the reattached adapter, but the radio
had already disappeared; the new process again starts without a WBLink. Its
camera pipeline resumes. Ground was not restarted or reconfigured in this run.
Air now reports `get_throttled=0x50000`: historical undervoltage/throttling flags,
with no current bits set. This does not prove the external radio supply is good.

The affected hub port's `disable` state reads 1 and `over_current_count` reads 0.
One targeted attempt to write 0 to the port's disable control fails with an I/O
error, and the adapter remains absent. No hub-wide reset or reboot was performed.
OpenHD never reached Devourer initialization on this resumed run.

The user reports the same failure across multiple 8812EU cards, cables, adapters,
and Pis. This shifts the investigation toward a shared software/initialization
cause; a physical defect is not established. NetworkManager logs confirm repeated
bring-up and scan-MAC changes on wlan1 between the USB disconnects while OpenHD
has no WBLink. The kernel driver therefore initializes the card during this gap.

## Kernel-driver isolation test prepared

Following the user's suggestion to remove the kernel driver, Air's unused
`88x2eu_ohd` module was unloaded successfully. Temporary blacklist file
`/run/modprobe.d/90-openhd-devourer-eu-diagnostic.conf` initially contained an
alias blacklist. This alone did not prevent loading after the subsequent hub
reset; the stronger temporary block used for the successful test is below.
The installed module/package remains available.

OpenHD's USB discovery code explicitly constructs a `devourer-usb-*` radio when
no kernel netdev exists. The kernel 8812EU driver is not required for that path.
A disconnect/reconnect including the adapter's external power has been requested
to clear its failed state before testing this path. No improvement from module
removal has yet been demonstrated: the adapter still does not enumerate.

An additional packaging mismatch was observed: the installed
`realtek_88x2eu.conf` names `rtl88x2eu_ohd`, whereas modinfo reports module name
`88x2eu_ohd`. The same mismatch exists in the KernelBuilder overlay. This is
separate from the unproven driver/NetworkManager race and has not been changed.

## Previous-commit comparison: no persistent CU hub-reset change found

The September 27 RTL8812CU experiments included USB/hub resets and an EEPROM
`USB_MSD_PWR_OFF_TIME=5000` test. The recorded experiment restored and verified
the original EEPROM configuration. The current Air bootloader configuration also
contains no `USB_MSD_PWR_OFF_TIME` override.

SysUtils commits `d3c57d7` and `6089f2b` implement storage-mode eject and retry
timing. They restrict the operation to Realtek `0bda:1a2b` with a mass-storage
interface and call `usb_modeswitch` for that device. They do not reset the hub or
host controller, and do not target the Air EU identity `0bda:a81a`. Installed
udev rules similarly target the storage identity. No persistent reset script or
service was found in the inspected installed configuration.

Comparing current OpenHD `9d998bcc` with October 3 `5da68fb2` shows discovery and
hotplug matching changes, not hub resets. Devourer's changes since October 3 do
not change UsbOpen/UsbTransport. The module-options naming mismatch predates the
CU experiments (KernelBuilder commit `8b9f600`, April 7). There is no identified
CU hub-reset source change to revert. No source rollback or binary deployment
was performed; recovery below uses the current installed binaries.

## Recovery and paired RF tests

After the requested physical power replug, the hub still failed to enumerate the
EU adapter. One USBDEVFS_RESET of the enumerated VIA hub at
`/dev/bus/usb/001/002` recovered enumeration as device 24. This was a diagnostic
action on that hub, not an installed automatic action. No xHCI-controller reset
or Pi reboot was performed.

The kernel module loaded again after that reset. Air's wlan1 was marked unmanaged
in NetworkManager, and the temporary module configuration was strengthened to:

```text
blacklist 88x2eu_ohd
install 88x2eu_ohd /bin/false
```

After `udevadm control --reload` and unloading `88x2eu_ohd`, the adapter remained
enumerated with no kernel netdev. Restarting Air OpenHD then initialized
`devourer-usb-1-24` as RTL8822E, and Air exposed its RF parameters again.

Both devices' radio settings were backed up to
`/root/connection-settings-backup-20261005/wifibroadcast_settings.json`.
The tested baseline was set to 5785 MHz, width 20 MHz, PIT disabled on both,
Air TX power level 20, Ground TX power level 30, Air MCS 2 and Ground MCS 0.
STBC/LDPC/short guard interval and the diagnostic UDP bridge remained disabled.
Restarting Ground OpenHD restored bidirectional RF traffic and RF video delivery.
The camera remained enabled; automatic encoder targets changed with link capacity.

Commands sent only to Air verified Ground following these changes without direct
Ground parameter commands:

- Frequency: 5785 -> 5745 -> 5785 MHz.
- Channel width: 20 -> 40 -> 20 MHz.

The changes caused brief interruptions and FEC losses, particularly when returning
to 5785 MHz. Reception recovered without another restart. At 40 MHz, incoming
video reached roughly 16.6 Mbps; this short test does not establish sustained
loaded 40 MHz performance. The final setting is 5785 MHz / 20 MHz.

The final 30-second steady-state capture
`out/wfb-telemetry-1791204934.json` measured Ground RF video at
11,061,729-11,170,834 bps, 1,825 additional FEC blocks, and zero additional lost
FEC blocks. The Ethernet video link counter remained zero. Air received uplink
packets and Ground received Air telemetry. Both services were active, both USB
adapters remained present, and both binary hashes remained unchanged.
TX injection-error hint counters still increased (Air +43, Ground +5), so the
short stable sample is not proof that every radio performance issue is resolved.
Decoded video and the rendered QOpenHD controls were not inspected.

Driver/NetworkManager initialization is a candidate contributor, but these tests
changed several conditions: hub state, kernel-driver ownership, settings, and
service startup. They do not isolate one definitive cause. A regression caused
by a persistent CU hub-reset change was not demonstrated.

## State after the initial manual recovery (superseded below)

The devices are connected using the latest installed OpenHD binaries. The Air
kernel-driver block exists only under `/run` and disappears on reboot; cold-boot
recovery has not been verified. No driver package was deleted or permanently
blacklisted. To undo the temporary block without rebooting, remove
`/run/modprobe.d/90-openhd-devourer-eu-diagnostic.conf` and reload udev configuration.
Any subsequent driver reload must be coordinated with the active Devourer radio.
The original settings backups remain available. No commit or push was performed.

## Packaged ownership fix and reboot validation

SysUtils now installs a persistent EU modprobe block and a USB-identity-specific
NetworkManager exclusion. Before discovery it releases interfaces still owned
by the legacy EU driver, after clearing NetworkManager ownership. It leaves
active Devourer usbfs claims alone. There is no automatic hub/controller reset,
USB power cycle, module unload, or EEPROM change in this fix.

Both normal and X20 SysUtils configurations built and passed the ownership test.
Staged packages include both policy files. Configuration with the ownership
policy disabled is rejected for every target.

An initial cross-built SysUtils binary passed symbol resolution but crashed on
`--version`; it was rejected before installation. A native build on Air passed
the test, `ldd -r`, and `--version`. Native build/debug tooling was installed on
Air for this investigation. The tested SysUtils package is installed as
`1.0.3.119+eufix20261005`, allowing a later `1.0.3.120` package to supersede it.
Its installed binary SHA-256 is
`aa5a9f052c34a572d937dab5b5df7e59be138d9a2978baeea559ca65fbf75e6c`.
The original SysUtils binary, service and configuration are backed up in
`/root/sysutils-eu-fix-backup-20261005/`. The configuration contents and OpenHD
binary were preserved.

The temporary `/run/modprobe.d/90-openhd-devourer-eu-diagnostic.conf` was removed
before rebooting Air. Boot ID `10c2bfcb-cb81-4897-9fbb-a369b658a72b` initialized
the EU adapter with Devourer, without loading the EU kernel module. Both services
were active. This verifies a software reboot, not a cold power-on.

The post-reboot 30-second RF sample `out/wfb-telemetry-1791207834.json` measured
roughly 11.02-11.15 Mbps at Ground, 1,828 additional FEC blocks and 11 lost blocks.
Air-only changes 5280 -> 5300 -> 5280 MHz and 20 -> 40 -> 20 MHz were accepted
and followed by Ground. Reception at 5300 MHz was poor; switching back recovered
it. These short checks verify parameter propagation, not reliable reception on
every channel or sustained 40 MHz operation. The final tested setting is
5280 MHz / 20 MHz.

## Remaining disconnect and process-abort problem

After this reboot the kernel initially recorded three further USB disconnects, followed
about five seconds later by OpenHD signal-6 aborts and systemd restarts. The EU
kernel module remained unloaded. Whether the user physically interrupted the
adapter during those events was not confirmed. Thus removing the competing
kernel driver does not establish that the recurring disconnect is fixed.

One controlled test deauthorized and reauthorized only the verified EU USB device
with a two-second interval. It did not reproduce the abort: OpenHD PID 25541 and
restart count 3 remained unchanged. The kernel reported error -71 and reset the
VIA hub during recovery; Devourer rejoined the radio after roughly 12 seconds.
The test did not issue a hub-reset command. RF video resumed without restarting
OpenHD. The subsequent eight-second sample `out/wfb-telemetry-1791210565.json`
recorded 496 additional FEC blocks, two lost blocks, and about 11 Mbps RF video.

The preexisting `/core` belonged to `rpicam-vid`, so it cannot explain OpenHD's
abort. A temporary process-specific core path captured no new core during the
controlled test; the original kernel core pattern was restored. The abort cause
and the recurring disconnect remain open. Decoded video and QOpenHD UI behavior
have not been inspected.

A subsequent health check recorded two more USB disconnects followed by signal-6
aborts, bringing the systemd restart count to five. The service was active again
with PID 11152 and the adapter enumerated as USB device 9, claimed by `usbfs`.
The EU kernel module remained absent. These later events reinforce that the
remaining failure is reproducible without the legacy driver loaded.

## Devourer-only image integration

OpenHD configuration now rejects a disabled or missing Devourer backend.
ImageBuilder removes the old broadcast radio package/module installation paths,
including X20 and the desktop x86 installer, purges existing matching packages
and modules during image preparation, and installs a persistent module block.
The Luckfox Buildroot recipe pins OpenHD 3.0 and its matching Devourer source;
the SDK no longer clones the EU kernel driver. Onboard networking remains
available. Image cleanup requires evidence of Devourer in the installed OpenHD
binary before removing packages.

Image cleanup fixtures, shell syntax, normal/X20 SysUtils builds and tests, and
OpenHD ARMHF CMake configuration passed. Buildroot extraction/finalization hooks
were exercised with a fixture, including rejection of a legacy binary before
module removal. No complete image build or release publication was performed.
The locally tested SysUtils package is deployed only on Air. The implementation
was committed and published as OpenHD `bce2ab8e` (openhd-3.0), SysUtils `0bcaaaa`
(dev-release), and ImageBuilder `2796a4f` (dev-release). Each remote branch was
fetched and verified against the local branch before publishing this report.

## Continued USB and abort investigation

The later read-only check found 13 OpenHD SIGABRT restarts in the same boot.
Kernel events show EU device `0bda:a81a` disappearing at port `1-1.2` before
those aborts, with a newly assigned USB address after each reconnect. The Pi
reported `get_throttled=0x0`; USB power control was already `on`. These checks
do not measure the adapter's external supply. The user confirmed that the card
was untouched and continuously powered during the continued monitoring run.
Initial evidence is `out/8812eu-disconnect-2026-10-05.log`.

An eight-second device deauthorization at device-local time 18:14:47 recovered
without a process restart. A separate eight-second USB-device driver unbind at
18:19:12 also avoided SIGABRT. Neither test is a physical removal: discovery
can still find the USB identity, and the second test even reopened the radio
before the parent USB-device driver was rebound. Both used verified VID/PID and
an exit trap to restore only the EU device; no hub-wide reset was requested.

The second test exposed a distinct receive-lifecycle defect. At 18:28 Air
reported zero RF receive packets while still injecting video, Ground continued
transmitting uplink packets, and Jaguar3's coex thread was draining bulk-IN.
Devourer's async RX loop can return normally on `NO_DEVICE`; the transport only
reported exceptions as fatal, leaving that normal early exit unreported.
Evidence: `out/wfb-telemetry-1791221319.json` and the saved console/USB traces.

A local wifibroadcast change reports unexpected normal RX-loop exits through
the existing recovery callback, with an atomic per-card stop request suppressing
that callback during deliberate teardown. The ARMHF OpenHD build completed.
This change has not been deployed or hardware-validated. Existing uncommitted
TX-failure diagnostic changes in the same submodule were preserved.

Air's OpenHD service was restarted at approximately 18:29 device-local time to
restore reception after the controlled test. PID changed from 26179 to 20843;
the explicit restart reset systemd's restart counter to zero, so it must not be
interpreted as disproving the earlier 13 aborts. The following eight-second
sample confirmed nonzero Air RX and approximately 11 Mbps RF video at Ground:
`out/wfb-telemetry-1791221379.json`.

Passive unique-name core capture is temporarily active under
`/tmp/openhd-usb-investigation/core.%e.%p`, with a watcher restoring the original
`core` pattern after the first OpenHD core or a 40-minute timeout. Filtered
usbmon errors and preceding control transfers are saved beside it. The earlier
GDB attachment was removed because its interception of frequent subprocess
launches changes timing. Devourer logging was enabled for diagnostics. At that
checkpoint the spontaneous disconnect and SIGABRT causes remained unproven.

## Captured spontaneous abort

At device-local time 18:39:55, with no manual USB operation during the soak,
the EU disappeared and OpenHD PID 20843 aborted. The passive watcher captured
`core.openhd.20843` and automatically restored the original core pattern. The
core identifies `/usr/local/bin/openhd`, not the camera process. The restarted
service was active with PID 30858 and restart count 1.

The filtered USB trace's first retained error is bulk-OUT endpoint 8 returning
`-71` with 512 bytes completed. Bulk-IN errors and a vendor control-read error
follow. Devourer recorded `bulk_send EP 8 FAIL rc=-1 got 512/1538`, followed by
`rtw_read(3d08), sizeof(T) = 4`. The preceding retained control transfers
completed successfully. This establishes a USB-level error before the abort;
it does not establish why the device developed that error.

GDB's automatic unwind of the aborting thread stops after libc `abort()`, so
the saved stack-memory dump is also required. Its contiguous caller return
addresses identify `UsbTransport::read32`, `Halrf8822e::bb_get`,
`Halrf8822e::read_thermal`, `RtlJaguar3Device::GetThermalStatus`,
`Transport::get_thermal_status`, `WBTxRx::get_devourer_thermal_status`,
`WBLink::wt_update_statistics`, and `WBLink::loop_do_work`. C++ exception and
verbose-terminate-handler addresses are also present. The failed `3d08` read
is the direct RF thermal register read, and the source throws
`std::ios_base::failure` on failure. No catch previously protected this
statistics-thread call path. This evidence identifies the process-abort cause.

The wifibroadcast thermal getter now catches that read exception and returns
an unavailable reading instead of terminating OpenHD. The ARMHF OpenHD build
passed after this change. Both recovery fixes remain undeployed; source/build
validation is not hardware acceptance. The cause of the initial USB protocol
error/disconnect remains open.

Private local evidence is in `out/8812eu-crash-evidence-2026-10-05/`, including
the OpenHD core, its exact unstripped executable, GDB output, console capture,
usbmon errors, kernel log, and service journal. The executable SHA-256 remains
`32f1657499500b9aa4043c1e25467e055817d93c3040a7923f31d54ed8bef35e`.
The usbmon collector was stopped and `WB_DEV_LOGS` restored to its original
disabled value after capture. Air remains on the original binary.
