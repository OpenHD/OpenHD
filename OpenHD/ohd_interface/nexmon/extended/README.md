# Pi 4 extended-channel reception experiment, v1

Targets **Raspberry Pi 4 Model B, ARM32 kernel 6.1.29-v7l+** only.
This is a built experiment, not hardware-verified extended-channel support.

The driver adds channel-table entries 169/173/177/181/185 (5845/5865/5885/
5905/5925 MHz). The firmware adds ioctl 512: inspect the PHY channel lookup,
attempt the original firmware `chanspec` setter, and return its actual result
and current chanspec. Channel 165 (5825 MHz) is the control measurement.
The PHY lookup wrapper is pinned to 7.45.206 at 0x1CCF20.

Firmware validation and PHY calibration are retained. Channels not advertised
by the firmware remain marked disabled in `iw`; the explicit probe operation
can attempt a direct firmware tune. Ordinary Scan/Analyse do not use this
bundle or bypass their enabled-channel checks. A successful probe still does
not establish reception or sensitivity. If the PHY has no matching channel
data or the setter rejects a candidate, we need those diagnostics before
developing a further firmware patch. This stage does not add extended 2.4 GHz
channels or claim the whole Devourer range works.

## Test setup

Use Ethernet for the Pi: its hotspot disconnects during a lease. Use a known
Realtek/OpenHD transmitter fixed to one candidate at a time, with FHSS off and
20 MHz management frames. Verify its channel on the SDR. The SDR must cover
5.825–5.925 GHz directly or through a suitable downconverter; a receiver limited
to lower frequencies will not cover this experiment. A receiving SDR observes
the transmitter, not the Pi's receive tuning. To prove Pi reception, inspect
the captured frames for the known transmitter and compare transmitter-off/on
captures. A CW tone alone does not test Wi-Fi packet reception.

First test 5825 MHz. If the baseline probe or capture fails, stop and resolve
that before interpreting extended channels. Keep transmitter power and spacing
constant across comparisons. Record SDR model, frequency, gain, sample rate,
transmitter settings and identity with the results. Keep the test transmitter
on the selected channel throughout each capture; repeat for each frequency.

## On-Pi commands

Extract the supplied tarball into a new directory, e.g.
`/opt/openhd-nexmon-extended-v1`, and enter it. Files are `bundle/`, the
experimental helper, test script and this README. Verify the bundle before use:

```sh
cd /opt/openhd-nexmon-extended-v1/bundle
sha256sum -c SHA256SUMS
cd ..
sudo systemctl stop openhd
# Transmitter off, then repeat with it transmitting on 5825 MHz:
sudo python3 test_nexmon_extended.py --helper ./openhd-nexmon-scout \
  --bundle ./bundle --phase off --frequencies 5825 --output ./5825-off
sudo python3 test_nexmon_extended.py --helper ./openhd-nexmon-scout \
  --bundle ./bundle --phase on --frequencies 5825 --output ./5825-on
# Set transmitter and SDR to 5845, repeat off/on with new output directories.
# Then test 5865, 5885, 5905 and 5925 the same way.
sudo systemctl start openhd
```

Each run leases the radio, produces `results.jsonl` and (only for a matching
tune/readback) radiotap `.pcap` files, then restores the stock radio/hotspot.
Output directories must be new. The independent 600-second restore timer
covers a crashed process. Keep the extracted directory in place until
restoration finishes, because the timer calls its helper there.

`phy_frequency_mhz` reports the firmware's PHY lookup, `firmware_set_return`
reports the original setter, and `current_chanspec` reports its readback.
`capture_ready` requires a successful setter, matching chanspec and frequency,
and matching `iw` readback. Rejected probes are recorded without capture;
missing/malformed firmware replies abort the test and trigger restoration.
`matching_frequency_good_fcs_packets` counts candidate-frequency packets without
the bad-FCS flag; it does not authenticate them or identify the transmitter.
`hardware_verified` remains false: acceptance requires inspecting the PCAPs
and the controlled off/on results.

Manual recovery, while the directory still exists:

```sh
sudo python3 ./openhd-nexmon-scout restore
sudo systemctl start openhd
```

## Rebuild and source

Run `scripts/build_nexmon_extended.py` on Linux with a new `--output`, exact
`--kernel-build` (including matching Module.symvers), absolute GCC 13.3
`--cross-compile` prefix, and Nexmon GCC 5.4 `--firmware-toolchain` directory.
Supply `--modpost` if the kernel headers' host modpost is incompatible. A wrapper
must forward stdin: modpost reads the module list from it. Host tools include
GNU make, GCC, gawk and Python 3.9+. The script uses Python zlib for microcode
compression and does not need zlib-flate.

The output contains the modified source, generated source diff, built bundle,
and a self-contained test tarball. Nothing is installed or selected automatically.
The source archive inside the bundle includes the modified driver and firmware
source and their original license notices. `openhd_extended_probe.h` is
GPL-3.0-or-later as part of Nexmon. The shared `structs.common.h` was omitted
from the original OpenHD archive and is supplied unchanged from upstream commit
`d6b633800d80b8b8e2132a2c9a5ecda870ec8aaa`:
https://github.com/seemoo-lab/nexmon/blob/d6b633800d80b8b8e2132a2c9a5ecda870ec8aaa/firmwares/bcm43455c0/structs.common.h
SHA256: `57aedee35fc717f02da0bf18a1fad9c4c5fe38682503b8f6907a2af4b9e9a134`.
The bundle retains `LICENSE.nexmon`; upstream materials are not relicensed.
