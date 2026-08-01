## Summary

The responsibilities of this module depend on weather it us used on air or ground.
# AIR
1) Detect connected cameras (can be manually overridden)
2) Setup a pipeline that generates a continuous stream of encoded video data (h264 or h265) for a detected camera
   This data is forwarded via a callback (previously udp port) to ohd_interface for transmission via wb
3) Store and change camera/encoder-related settings
   Camera settings are stored in SETTINGS_BASE_PATH/video (one unique file for each camera)
   The Camera manifest (list of discovered cameras) can be found under /tmp/camera_manifest for debugging.

# Ground
1) Forward the received video data via RTP, UDP to other consuming applications 
   (e.g. QOpenHD for display). Those application(s) can run on localhost, or on externally connected devices
   (e.g. android smartphone via USB tethering)

## Note 
The code in this module must adhere to the following paradigms:
1) It only generates encoded video data,then forwards it. It doesn't know if the video data is actually picked up or makes it to the
   ground.
2) A camera and it's settings are located on the air unit - you can change / query param(s) via mavlink from the ground unit, but we keep the ground
   completely agnostic to the camera(s) and their settings for a good reason !
3) There are no code dependencies to other modules like ohd_interface.
4) to adhere with 1), for h264/h265 streaming, re-send the "configuration data" (aka SPS,PPS,key frame for h264, SPS,PPS,VPS,key frame for h265)
in regular intervals. This way the decoding application can start the video decoding after max. 1 interval size, assuming a connection
without packet drops

## Managed IP cameras

Select camera type `EXTERNAL_IP` (3) as either camera, then set that component's
`IP_CAM_PIPELINE` extended MAVLink parameter. The value is the source portion
of a GStreamer pipeline and must produce an elementary stream matching
`VIDEO_CODEC`. OpenHD appends the normal performance monitoring, recording,
RTP packetization, encryption and radio-link stages and restarts the camera
pipeline automatically when the parameter changes.

An IP camera in either slot gets this T010-compatible H264 default:

```text
rtspsrc location=rtsp://{IP}:554/stream=0 latency=0 ! rtph264depay
```

`{IP}` is replaced with the per-camera `IP_CAM_ADDRESS`, which defaults to
`192.168.144.108` for the T010 test. The address is editable through MAVLink
and each supported GUI. When its `/24` is not already configured, OpenHD adds
a secondary local address (host `.20`, or `.21` if required). It does not
replace the existing Ethernet address, routes, or NetworkManager profile. The
pipeline is persisted and limited to 127 bytes by MAVLink. Clearing it restores
the legacy RTP input on UDP port 5500. Hotspot clients can use the configured
address to open the camera web interface.

OpenHD reserves 2 Mbit/s for the IP camera by default. Change
`V_IP_CAM_MBITS` to adjust this fixed reservation. OpenHD cannot control the IP
camera encoder, so configure the camera itself to stay at or below the reserved
rate and keep it low. The remaining recommended video bitrate is assigned to
the managed non-IP camera.
