# bin/bash
# If video shows up after some time, the main stream is working ;)

gst-launch-1.0 -v udpsrc port=5620 caps = "application/x-rtp, media=(string)video, encoding-name=(string)mjpeg" ! rtpjpegdepay ! decodebin ! videoconvert ! autovideosink