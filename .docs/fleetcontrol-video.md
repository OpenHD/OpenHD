# FleetControl connection and video outputs

FleetControl uses one craft identity with separate Air and optional Ground
WireGuard profiles. Each device needs its own key and address. In QOpenHD, open
OpenHD → FleetControl, sign in, create/select the craft and connect Air. Add Ground
to that same craft. Existing license/certificate controls remain available.

The HTTPS setup service on device port 8443 authenticates the `openhd` OS account.
QOpenHD asks to trust its certificate, then pins it. Profile installation validates
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
