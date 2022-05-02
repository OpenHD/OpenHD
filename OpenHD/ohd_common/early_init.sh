#!/bin/bash

export PATH=/usr/local/bin:${PATH}

source /usr/local/share/wifibroadcast-scripts/global_functions.sh


# this is temporary until the video system in QOpenHD has zero-copy enabled. this is not hard on the older raspberry
# pi broadcom stack, but has changed significantly on the newer KMS/DRM stack
detect_os
configure_hello_video_args
/usr/local/bin/hello_video.bin.48-mm ${HELLO_VIDEO_ARGS} &

exit 0
