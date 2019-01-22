#!/bin/sh

gst-launch-1.0 videotestsrc ! videoconvert ! video/x-raw,format=NV12,framerate=60/1,width=1280,height=720 !  vaapih264enc bitrate=4000 keyframe-period=3 rate-control=cbr max-bframes=0  ! rtph264pay config-interval=-1 ! udpsink host=localhost port=5602
