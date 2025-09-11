# MPP based video streaming

OpenHD traditionally uses a GStreamer pipeline to capture and encode video
before forwarding the resulting RTP stream through the wifibroadcast link.  On
Rockchip based platforms an alternative approach is now available which avoids
GStreamer entirely.  Instead the Rockchip **MPP** (Media Processing Platform)
encoder is invoked directly.

When `use_mpp_video` is enabled in `air_camera_generic.json` the video module
spawns an external `mpp_stream` process.  The tool is expected to output an
H.264/H.265 elementary stream to `stdout`.  OpenHD reads this byte stream,
extracts NAL units and packetises them into RTP fragments using the existing
`RTPHelper` infrastructure.  The resulting fragments are then forwarded via the
wifibroadcast link just like in the GStreamer based workflow.

The command line used to start the encoder is built from the current camera
settings (resolution, frame‑rate and bitrate):

```
mpp_stream --width <w> --height <h> --fps <f> --bitrate <b> --output -
```

Make sure the `mpp_stream` utility is installed on the system and supports these
parameters.  Any additional arguments can be provided by adjusting the command
construction inside `mpp_stream.cpp`.

This mode is still experimental and does not yet support dynamic bitrate changes
or onboard recording.  It is intended as a minimal example showing how OpenHD
can integrate with MPP without relying on GStreamer.
