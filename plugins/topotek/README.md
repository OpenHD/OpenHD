# OpenHD Topotek plugin

This air-side runtime plugin integrates Topotek SIP/HI-family Ethernet gimbal
cameras with OpenHD. It uses Topotek's ASCII UDP protocol on camera port 9003
(client reply port 9004) and is intended to accompany the camera's RTSP main
stream.

Supported controls:

- OpenHD stream resolution and bitrate changes. Topotek exposes 3840x2160,
  1920x1080, 1280x720, and 640x480, with bitrate steps from 1024 to 8192 kbit/s.
  Higher OpenHD link targets are clamped to the camera's 8192 kbit/s maximum.
- pitch/yaw/roll rate, angle, center/reset, calibration, lock, and follow commands
- continuous optical zoom, focus, and autofocus
- photo capture and explicit start/stop recording
- main/secondary (EO/IR) view modes and thermal palettes

Absolute/percentage optical zoom, FPV gimbal mode, codec, and frame-rate changes
are not exposed by the documented Topotek SIP protocol and are reported as
unsupported.

## Build and install

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
sudo cmake --install build
```

The install step places `libopenhd_topotek.so` in
`/usr/local/lib/openhd/plugins`. `package.sh` builds a Debian package under
`/usr/lib/openhd/plugins`.

## OpenHD camera settings

Configure an external IP camera with settings equivalent to:

```text
IP_CAM_ADDRESS=192.168.144.108
IP_CAM_PIPELINE=rtspsrc location=rtsp://192.168.144.108:554/stream=0 latency=0 protocols=tcp ! rtph264depay ! h264parse config-interval=-1 ! appsink drop=true
VIDEO_CODEC=H264
```

The exact persisted setting names and pipeline syntax depend on the OpenHD
image version. The camera used during development advertised H.264 video and
AAC audio at `rtsp://192.168.144.108:554/stream=0`.

Optional service environment overrides:

```text
OPENHD_TOPOTEK_IP=192.168.144.108
OPENHD_TOPOTEK_PORT=9003
OPENHD_TOPOTEK_CLIENT_PORT=9004
OPENHD_TOPOTEK_ANGLE_SPEED=50
```

`OPENHD_TOPOTEK_IP` is optional; by default the plugin follows OpenHD's
`IP_CAM_ADDRESS`. Angle speed is a raw protocol value from 1 to 99 (0.1 degree
per second units).

Topotek replies to UDP commands, but this initial plugin deliberately treats
commands as fire-and-forget so the OpenHD control path never blocks on a camera
firmware response.
