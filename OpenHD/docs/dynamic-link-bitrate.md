# Dynamic Link Bitrate

## Changed components

OpenHD:
- ohd_common/inc/openhd_bandwidth_allocation.hpp: per-link-instance budgeting and snapshots.
- ohd_common/inc/openhd_link_usage.hpp: non-consuming sampled traffic lookup.
- ohd_common/inc/openhd_action_handler.h and ohd_interface/src/wb_link.cpp: existing controller integration and disable restoration.
- ohd_video/src/ohd_video_air.cpp and its header: consumer discovery, liveness and target application.
- CameraStream, GStreamerStream, CameraProcess and LibcameraAppStream headers: existing minimum bitrate constraints.
- ohd_telemetry/src/internal/OHDMainComponent.cpp: optional BWA1 snapshot telemetry.
- settings/catalog.json and generated metadata: updated existing parameter labels/descriptions.
- ohd_common/test/test_bandwidth_allocation.cpp and CMake target: allocator regression.

QOpenHD:
- LinkUsageMeter.qml, LinkSegmentsBar.qml and LinkAllocationPopup.qml: usage and allocation presentation.
- LinkQuickPanel.qml and qml.qrc: existing section and resource integration.
- CameraDualPanel.qml, CameraSettingsPanel.qml and MavlinkParamPanel.qml: remove duplicate controls/navigation entries.
- CameraConnectionPanel.qml: suppress the legacy fixed reservation while dynamic allocation is enabled.
- app/telemetry/models/aohdsystem.cpp: BWA1 decoding with legacy/truncation handling.
- app/telemetry/settings/catalog.json: matching existing parameter descriptions.
- tests/link_allocation and tests/ethernet_observations_test.cpp: UI and wire-decoder checks.

## Integration and persistence

`VARIABLE_BITRATE` remains the persistent Air WiFi controller switch. The single
WBLink MCS/FEC recommendation feeds the allocator in OHDVideoAir; no additional
rate-control thread, TX-drop reduction, or loss-based controller is introduced.
Disabling restores each controllable encoder's saved manual rate as a temporary
capacity-limit request. It never rewrites BITRATE_MBITS, camera type, or stream settings.
The existing V_PRIMARY_PERC preference in air_camera_generic.json retains the two
camera proportions (10–90 percent supported by the existing firmware parameter).
A single live controllable camera gets 100% without overwriting that preference.

The total Link Usage section owns the toggle, clickable bar and popup. Camera
and raw Air pages no longer duplicate the Variable Bitrate editor. The manual
IP-camera reservation editor remains for legacy/manual operation and is hidden
while dynamic allocation is enabled.

## Transport and consumer model

`openhd_bandwidth_allocation.hpp` defines Transport, Consumer, Priority, Control,
Allocator and a synchronized snapshot Store. Each transport type and instance has separate history;
no capacities are added. Current integration allocates WiFiBroadcast only. UART
offloading, link aggregation and automatic consumer migration are not implemented.
The existing usage registry provides a non-consuming, three-second-fresh view of
its sampled per-category counters, so the telemetry sender remains the only sampler.
Priority traffic is the telemetry/control actually sent on this transport. Audio
and miscellaneous payloads, plus active encoders without advertised live-bitrate
support, are fixed consumers. Capability comes from CamInfo.supports_variable_bitrate,
not camera-type guesses or merely detecting a plugin. Camera liveness is based on
actual encoded-frame callbacks with a three-second timeout. Restarted/disconnected
streams are removed from the proportional pool. Measured fixed traffic includes IP
cameras automatically; an active fixed stream without a sample leaves allocation
unavailable rather than inventing a rate.

## Capacity, reservations and constraints

Capacity starts with the existing configured MCS/channel-width budget, after the
existing UDP-data reservation and FEC deduction. This is an estimated usable payload
capacity, not a measured speed test. We do not sum independent transports.

Priority reserve: measured priority traffic times 1.25.
Fixed reserve: measured fixed traffic times 1.15.
Safety/management: max(64 kbit/s, 3% of usable capacity).
Dynamic pool: max(0, capacity - priority - fixed - safety).
Camera pool share: dynamic pool times the normalized camera percentage.

Reservations have immediate attack and slow release (15%/10% new sample weights).
Encoder target increases are limited to 20% per second with a 5% / 100 kbit/s dead
band; reductions and explicit share changes apply immediately. Existing encoder
minimums, hardware maximums and saved manual ceilings constrain targets. GStreamer
and libcamera-specific minimums are reused through CameraStream's constraints API,
including supervised workers. The snapshots expire after three seconds without an
allocation update. Percentages remain unchanged at zero capacity. If minima or
fixed traffic exceed capacity the snapshot reports overcommitment; it does not
pretend a negative pool or an impossible zero-rate encoder is achievable. Unmanaged
fixed streams are never reconfigured or paused.

The popup distinguishes measured traffic, estimated reserves, calculated share
budgets and applied capacity-limit targets. The bar clips at 100% and reports
overcommitment separately instead of renormalizing overload to look safe. Actual
consumer throughput can overshoot estimates for a variable external camera; this
allocator is not an on-wire traffic policer.

## Telemetry compatibility

DATA96/OHLU version 1 and its first 46 bytes remain unchanged. WiFi samples optionally
append `BWA1` followed by seven little-endian uint32 fields: capacity, priority reserve,
fixed reserve, safety reserve, pool, measured fixed traffic (all kbit/s), and flags.
Flags: available=1, overcommitted=2, waiting=4, enabled=8. Each of two stream entries
then holds active/controllable flags, uint8 percent, uint32 applied budget in kbit/s.
Total payload length is 90. Old clients ignore the trailer; new clients show unavailable
allocation for old firmware and keep the legacy toggle's meaning.

## Validation and limits

- `test_bandwidth_allocation`: proportional totals, fixed reservations, single/disconnected
  cameras, unknown samples, zero capacity, ceilings and independent transport assignments.
- QOpenHD `tests/link_allocation`: toggle writes, both popup entry points, keyboard slider
  writes, shared percentages, single/fixed streams, preview write blocking, small windows,
  unavailable telemetry and the 100% cutoff. Runs on Qt 5 and Qt 6.
- `tests/run_ethernet_observations_test.py`: production telemetry decoder, including BWA1
  snapshots, legacy/truncated packets, missing fields and expiry.

Physical radio/camera adaptation, RF performance, and a complete image build still require
validation on the relevant hardware. Snapshot targets reflect requested limits; actual
encoder compliance is shown separately by measured throughput.
