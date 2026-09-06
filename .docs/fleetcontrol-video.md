# FleetControl connection and video outputs

FleetControl uses one craft identity with separate Air and optional Ground
WireGuard profiles. Each device needs its own key and address. In QOpenHD, open
OpenHD → FleetControl, sign in and assign an existing craft to a certificate.
The Start/Stop transmission button controls that craft's FleetControl video and
telemetry settings. Start enables licensed telemetry and primary video only;
Stop disables all streams. The radio video link is unaffected.

The HTTPS setup service on device port 8443 authenticates the `openhd` OS account.
The setup service is separate from certificate assignment. Profile installation validates
the device role and account routes, rejects wg-quick execution hooks, activates
WireGuard and restarts SysUtils/OpenHD. Do setup before flight. An old Air binary
without the independent video output is rejected before profile creation.

## Video

The FleetControl rendition is primary camera only, H.264, 854×480 (16:9 480p),
15 fps, 1000 kbit/s CBR target. Aspect ratio is preserved with borders. RTP uses
payload 96, 1200-byte MTU and SPS/PPS on each keyframe. Transport is UDP inside
WireGuard; RTP/IP/WireGuard overhead is additional to the encoder bitrate.

`VideoOutputProfile` describes an independent rendition and UDP destination.
`GstVideoOutput` attaches to the raw input of an existing encoder when possible,
sharing capture but running a separate encoder. Integrated encoded sources use
an encoded-input decode fallback. Queues are bounded and never block the primary
camera pipeline. Errors restart only the additional output. Camera restarts
destroy/recreate outputs; record-only operation does not upload.

Raspberry Pi uses x264 ultrafast/zerolatency. Other GStreamer platforms try a
separate Rockchip MPP encoder when available, then fall back to software on
failure. Hardware multi-encoding has not been tested on a Rockchip board. Native
MPP-only camera implementations do not yet attach this GStreamer output; they
need a raw-frame adapter or a native second encoder implementation. They must not
fall back to uploading the full-resolution radio stream.

MultiLink excludes LTE from primary video fanout. Air and Ground suppress
external-device forwarding to their native FleetControl destination, avoiding a
second upload when a telemetry TCP client connects. The existing native telemetry
transport remains MAVLink/UDP; legacy TCP telemetry is supported separately by
FleetControl with per-connection frame reassembly.

GStreamer-enabled OpenHD Ground builds also feed received primary H.264 RTP into
the independent output. Ground only encodes/uploads with permission from
FleetControl. Air's arriving video takes priority, regardless of the order in
which profiles were created. After three seconds without Air RTP, Ground may
resume. FleetControl labels the actual source as **Air video** or **Ground video**
and keeps one browser stream for the craft.

Both Air and Ground renew permission every 750 ms over the WireGuard UDP video port.
The 16-byte request is `OHDFV1Q\0` followed by eight random bytes. The reply echoes
the nonce with `OHDFV1Y\0` (allow) or `OHDFV1N\0` (pause). Only the account's known
Air or Ground peer can request permission. Stopping transmission denies both;
Air retains priority when transmission is enabled. Permission expires after two
seconds without a matching reply; the FleetControl encoder and upload stop,
retaining radio video for local consumers. Small permission queries, tunnel
keepalives and the telemetry control endpoint remain available while stopped.
Older Air firmware needs updating to stop its video upload at the sender. A brief
overlap during detection/permission propagation is possible; the server accepts
only Air as soon as Air RTP arrives.

`test/relay_video_output.cpp` provides the same output/permission behavior for
bench testing older firmware without replacing the OpenHD binary. Run it with
`HOST PORT INPUT_PORT air|ground`. The bench receiver and native Ground path
currently expect H.264 input. The generic output supports explicitly configured
H.265 input, but automatic Ground codec detection is not implemented.

## Verification

On the Raspberry Pi 4 at 192.168.1.124, the standalone test used a 1280×720/30
synthetic primary source while OpenHD Ground remained running. Decoded UDP output:

| Input | Resolution | FPS | RTP bitrate |
| --- | --- | --- | --- |
| Shared raw frames | 854×480 | 15.14 | 1020 kbit/s |
| Encoded fallback | 854×480 | 15.20 | 1024 kbit/s |

The Pi was subsequently connected to production FleetControl with a native
Ground profile. A continuous synthetic source using `GstVideoOutput` uploaded
over WireGuard/UDP and played in the browser's Video Matrix. The HLS playlist
reported 854x480, 15 fps and approximately 1.00 Mbit/s. Disabling x264 sliced
threading fixed green/corrupt browser frames that were absent in local libav
decoding; the subsequent browser sample decoded 361 frames without drops.

This validates the independent output and production video path. The running
OpenHD binary was not replaced or switched from Ground to Air, so integrated Air
camera startup and paired Air/Ground operation still require a firmware test.
The full OpenHD application is built by CI.

Subsequent paired tests used Ground 192.168.1.124 and Air 192.168.1.42 with
separate native WireGuard profiles and bench receivers. Ground upload while Air
was active fell to 720 bytes over eight seconds (permission/tunnel traffic,
no video). The direct RTP input used by native Ground separately decoded at
15.21 fps and 1017 kbit/s on the Pi. Native firmware integration is built by CI;
the bench devices still run their existing OpenHD binaries with test receivers.
