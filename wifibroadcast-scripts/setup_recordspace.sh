#!/bin/bash

source /usr/local/bin/wifibroadcast-scripts/global_functions.sh

detect_memory

# todo: these should not be top level root folders, they should either be in /tmp or /var/run/openhd


# telemetry is stored separately from video to avoid affecting telemetry recording when video space
# fills up
mkdir -p /wbc_tmp
mount -t tmpfs -o size=15024K tmpfs /wbc_tmp
mkdir -p /wbc_tmp/rssi/



mkdir -p /video_tmp

# use 1/3 of available ram by default for /video_tmp, which is used for recording telemetry and video
# when VIDEO_TMP=memory. We need to do this to avoid crashes or safety issues caused by running out of
# memory, which is easy to do when the ground station has just 512MB ram to start with and has 128MB set
# aside for the GPU, like the Pi3a+
available_for_video_tmp=$((${TOTAL_MEMORY} / 3))

# add a little extra margin 
available_for_video_tmp_final=$((${available_for_video_tmp} + 30000))

mount -t tmpfs -o size=${available_for_video_tmp_final}K tmpfs /video_tmp

VIDEOFILE=/video_tmp/videotmp.raw

echo "VIDEOFILE=${VIDEOFILE}" > /tmp/videofile

qstatus "Saving video to memory" 5
