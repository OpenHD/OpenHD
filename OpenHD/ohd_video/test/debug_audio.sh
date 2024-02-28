# bin/bash

#gst-launch-1.0 -v udpsrc port=5610 ! "application/x-rtp,media=(string)audio, \
#clock-rate=(int)44100, width=16, height=16, encoding-name=(string)L16,\
#encoding-params=(string)1, channels=(int)1, channel-positions=(int)1, \
#payload=(int)96" ! rtpL16depay ! audioconvert ! autoaudiosink sync=false

gst-launch-1.0 udpsrc port=5610 caps="application/x-rtp, media=(string)audio, \
 clock-rate=(int)8000, encoding-name=(string)PCMA" ! rtppcmadepay ! \
 audio/x-alaw, rate=8000, channels=1 ! alawdec ! autoaudiosink sync=false