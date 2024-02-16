# bin/bash
# If video shows up after some time, the main stream is working ;)

gst-launch-1.0 -v udpsrc port=5600 caps = "application/x-rtp, media=(string)video, encoding-name=(string)H264, payload=(int)96" ! rtph264depay ! decodebin ! videoconvert ! autovideosink sync=false