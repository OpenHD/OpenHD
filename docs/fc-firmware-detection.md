# MAVLink-only flight controller identification

`OHDMainComponent` owns `FcFirmwareDetector`, alongside the existing FC ID,
arming and RC handling. `AirTelemetry::on_messages_fc()` still forwards every
received message unchanged. Detection observes the same messages; the existing
100 ms air telemetry loop polls for bounded requests and sends them through the
existing serial endpoint. No MSP, USB discovery, IO worker, sleeps, or command
ACK waits are involved in identification.

## Evidence and confidence

The stable enum is `UNKNOWN=0`, `ARDUPILOT=1`, `PX4=2`, `INAV=3`,
`BETAFLIGHT=4`. Confidence is separate from the enum:

| Evidence | Result | Confidence |
| --- | --- | --- |
| PX4 vehicle heartbeat, autopilot 12 | PX4 | identified |
| Autopilot 3 and a nonempty, well-formed parameter response | ArduPilot | identified |
| Empty parameter signature: zero count, index and type, all-zero ID | INAV | likely |
| Generic vehicle heartbeat and a plausible nonzero firmware version | Betaflight | candidate |
| No reply, unsupported commands, malformed replies or conflicting evidence | Unknown, or retain existing positive candidate evidence | inconclusive / candidate |

Generic heartbeats, version numbers and empty parameter signatures are not
unique firmware fingerprints. No version is invented for INAV 8/9, and older
Betaflight without a version response remains unknown. Names never grant write
capabilities. GCS, companion, camera and gimbal heartbeats are ignored. An FC
heartbeat must use component AUTOPILOT1, legacy SYSTEM_CONTROL, or the legacy
system/component pair 0/200 (which alone is not firmware identification). Air uses the
configured FC system ID, also allowing the existing Betaflight system ID 0.

For ambiguous FCs, the state machine requests `AUTOPILOT_VERSION` with
`COMMAND_LONG / MAV_CMD_REQUEST_MESSAGE`, then `PARAM_REQUEST_LIST` if still
ambiguous. PX4 also gets a version request for its version display, without a
parameter scan. Response matching requires both FC system and component IDs;
parameter evidence is accepted only after a parameter request. Delayed replies
within the same session can improve the result, and duplicate replies don't
restart requests. Each stage sends at most one initial request and two retries.

Read-only identification probes use the conventional GCS sender `255/190`,
matching QOpenHD. The live INAV FC ignores requests from OpenHD's onboard
computer sender `101/191` (and mixed `255/191` or `101/190` addresses), but
answers the same request from `255/190` over either MAVLink 1 or 2. Responses
still must originate from the selected FC system and component. Normal
OpenHD status traffic retains its own sender identity.

A deployed FC may have the stream group containing HEARTBEAT disabled while RC
and temperature data continue. In that case, a valid RC/attitude/system-status
packet on the configured FC UART supplies only the address for delayed, bounded
read-only probes. It does not identify a firmware. An actual empty parameter
signature can still identify INAV; silence remains Unknown. A version reply
alone does not identify Betaflight without its generic heartbeat. Companion
endpoints are excluded from this fallback as well.

The following optional keys in telemetry `air_settings.json` configure detection
at OpenHD startup. Defaults are used when absent:

| Key | Default | Accepted runtime range |
| --- | --- | --- |
| `fc_detection_version_timeout_ms` | 1500 | 250–30000 |
| `fc_detection_parameter_timeout_ms` | 2000 | 250–30000 |
| `fc_detection_disconnect_timeout_ms` | 10000 | 4000–60000 |
| `fc_detection_retries` | 2 | 0–2 |

The detector uses a monotonic clock. Cache invalidation occurs on loss of all
matching FC traffic, heartbeat identity changes, BOOT transitions, changed
firmware versions, changed configured FC ID, or a significant backwards uptime
step within the same message type. Small reordering and 32-bit uptime wrap are
tolerated. Silence is never used to identify a firmware family.

## Status and QOpenHD

The existing parameter provider registers two hidden, read-only information
parameters, `FC_FW_INFO` and `FC_FW_EVID`, and broadcasts them every two seconds
using standard `PARAM_EXT_VALUE`. They retain the provider's actual parameter
count and index. INFO contains enum, confidence, version, system/component and
session generation; EVID contains matching generation/type and evidence. This
uses the normal Air/ground telemetry forwarding and parameter metadata APIs,
without a new MAVLink dialect or a second detector on Ground/QOpenHD.

QOpenHD consumes these values only from OpenHD Air's onboard-computer component,
validates the target FC, retains the last identity and observed fields during
packet loss, and clears observed fields when a new FC session is reported.
Evidence from another generation/type is discarded. Each incoming FC message
updates its readings immediately. A timer updates ages and packet rates only;
it does not clear values or recreate the telemetry tiles. Old values are
retained with their last-received age and an offline connection indicator.
Firmware status also passes through the parameter transaction handler, so its
two registered indexes complete normal parameter-list fetches. Unsolicited
updates do not start another fetch.
The craft Status card remains accessible after a heartbeat loss. The details
view distinguishes telemetry activity from heartbeat health and themes itself
for INAV, Betaflight, PX4, ArduPilot, or Unknown. The menu uses plain firmware
names; confidence and evidence remain internal diagnostics and logs. INAV home
compatibility is automatic: `GPS_GLOBAL_ORIGIN` supplies the home location for
detected INAV, including an origin received before identification completes.
The manual INAV compatibility toggle is removed. Compatibility home data is
cleared on session/family changes. Packet rates describe what reaches QOpenHD,
not configured FC rates.

Tiles are built from actual observed fields, with MAVLink sentinels and invalid
values excluded. Unsupported fields have no tile; old readings retain their
value and show their age. GPS navigation values require a fix. Battery temperature's
`INT16_MAX`, out-of-range throttle and unknown RSSI are not shown as real values.
Sensor health and recent events come from messages actually received. Firmware
identity/board/UID rows appear only when supplied. The craft view has no USB/MSP
beeper or refresh controls and does not guess support for reboot/calibration.
USB-to-FC choices were removed from the FC UART settings metadata; other OpenHD
USB functionality is outside this change.

## Validation

`test_fc_firmware_detector` runs deterministic protocol scenarios covering INAV
8/9-style signatures, old/no-reply and newer/version-reporting Betaflight,
ArduPilot, PX4, timeouts, packet loss, bounded retries, wrong senders, malformed
and conflicting evidence, duplicates, cache expiry, reboot and identity changes.
These are wire-behavior fixtures, not claims of hardware coverage of each release.

With the Linux build configured:

```sh
cmake --build <build> --target test_fc_firmware_detector
ctest --test-dir <build> -R '^fc_firmware_detector$' --output-on-failure
```

The header-only detector can also be tested independently:

```sh
g++ -std=c++17 -Wno-address-of-packed-member \
  -IOpenHD/ohd_telemetry/src \
  -IOpenHD/ohd_telemetry/lib/mavlink-headers/mavlink/v2.0 \
  OpenHD/ohd_telemetry/test/test_fc_firmware_detector.cpp -o /tmp/test_fc_firmware_detector
/tmp/test_fc_firmware_detector
```

QOpenHD interaction/render tests are in `tests/craft_details`. They cover
keyboard/mouse entry, status loss, tab navigation, expansion retention, dynamic
field omission, stale values, firmware labels and palettes.
