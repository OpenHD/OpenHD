# Passive Devourer channel survey

Analyse now surveys supported channels in 20 MHz bins using Devourer's
frame-free NHM and CLM measurements. Injection is paused once before the
sweep. Each successful tune settles for 30 ms, then takes five short CCX
readings spaced 20 ms apart. Histogram reductions are weighted by sample
count; channel-busy readings are weighted by their hardware window length.
The normal receive-quality poll cannot drain those counters during a dwell
because the survey holds the card's control mutex.

This replaces the old four-second packet-count observation on the Devourer
path. The observation budget is roughly 130 ms per channel plus tuning and
USB read time. Actual scan speed and interference detection on AU/EU have
not been measured. Short samples can miss intermittent transmitters, and
the CLM samples cover short hardware windows rather than every microsecond
of the entire dwell. Repeat a scan before treating a low reading as clear.

The configured frequency/width and passive-mode preference are restored
after the sweep. If restoration fails, injection remains paused and an
error is logged. Existing ESP/Nexmon analysis paths are retained.

## Reading the chart

QOpenHD offers energy above the noise floor (NHM environment ratio),
channel-busy percentage (CLM), Wi-Fi packet counts, and relative packet
counts. Higher energy/busy bars are evidence against using a channel.
Energy can be present without any decodable Wi-Fi packets. Neither an
invalid measurement nor a failed tune is reported as a zero-percent energy
reading: it is labelled N/A. No absolute dBm noise floor is invented for
EU hardware. For a 40 MHz link, inspect both constituent 20 MHz bins.

NHM/CLM see all received energy, including a running OpenHD Air transmitter.
The ground's own injection is paused, but the scan does not stop Air video.
Packet counts exclude valid OpenHD frames; energy does not. For a survey
of external interference alone, stop Air transmission first. The chart
explains this distinction instead of labelling occupied channels universally
bad or claiming that a zero reading guarantees a free channel.

## MAVLink contract

Message 1224 retains its generated layout, CRC, and foreign-packet fields.
The reserved fields use a new discriminator, separate from the existing
ESP discriminator `0x4553`:

- `dummy0 = 0x4456`: Devourer survey format v1.
- `dummy1`: batch identifier.
- `dummy2`: total channel count in the upper 16 bits, page offset below.
- `dummy[i]`: bit 15 NHM valid; bit 14 CLM valid; bits 7–13 NHM environment
  percentage; bits 0–6 CLM busy percentage. Valid percentages are 0–100.
  `0x3fff` indicates a failed tune/observation, including unavailable packet counts.
- `foreign_packets[i]`: the actual count observed during the shortened dwell.

Cumulative pages are repeated during progress updates, covering band lists
longer than 30 channels. QOpenHD combines pages by batch/source and keeps
packet counts separate from the energy measurements. Older clients ignore
the new reserved-field format and still receive packet counts.

## Validation

The Qt test exercises interference without decoded packets, quiet versus
unavailable readings, malformed percentages, separate packet counts, and
the actual popup at 1004×620 and 430×728. Rendered previews are generated
under `QOpenHD/build-channel-survey-test/release/`.

The statistics contract regression checks the survey's encoded MAVLink
fields and unchanged message length, alongside the existing ESP format.
No live radio scan, deployment, or RF range claim is implied.

Qt validation uses `tests/channel_survey_test.pro` with Qt 5.15.19. On
Windows the offscreen renderer needs `QT_QPA_FONTDIR=C:\Windows\Fonts`
and `QT_QUICK_BACKEND=software`. The five QtTest checks passed, and the
full Windows 32-bit QOpenHD build passed. Both preview sizes were inspected.

The Linux build uses the existing `OpenHD/build-codex-devourer` cache with
`CMAKE_ARCHIVE_OUTPUT_DIRECTORY=/home/damien/.cache/openhd-survey-archives` to avoid the
Windows volume's archive-copy space limit. Interface/telemetry compilation
and the statistics contract executable are checked separately from RF use.
`OHDInterfaceLib`, `OHDTelemetryLib`, and `test_statistics_contract` built
successfully; the statistics contract executable passed with assertions
enabled. The Wifibroadcast transport change is committed locally as
`aadee38` on `codex/passive-channel-survey`, and the parent pin follows it.
Publish the transport revision before the parent OpenHD pin. Device deployment
and live RF validation are separate from these source changes.
