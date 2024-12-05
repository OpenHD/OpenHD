# bin/bash

################################################################################
# OpenHD
# 
# Licensed under the GNU General Public License (GPL) Version 3.
# 
# This software is provided "as-is," without warranty of any kind, express or 
# implied, including but not limited to the warranties of merchantability, 
# fitness for a particular purpose, and non-infringement. For details, see the 
# full license in the LICENSE file provided with this source code.
# 
# Non-Military Use Only:
# This software and its associated components are explicitly intended for 
# civilian and non-military purposes. Use in any military or defense 
# applications is strictly prohibited unless explicitly and individually 
# licensed otherwise by the OpenHD Team.
# 
# Contributors:
# A full list of contributors can be found at the OpenHD GitHub repository:
# https://github.com/OpenHD
# 
# © OpenHD, All Rights Reserved.
###############################################################################

#gst-launch-1.0 -v udpsrc port=5610 ! "application/x-rtp,media=(string)audio, \
#clock-rate=(int)44100, width=16, height=16, encoding-name=(string)L16,\
#encoding-params=(string)1, channels=(int)1, channel-positions=(int)1, \
#payload=(int)96" ! rtpL16depay ! audioconvert ! autoaudiosink sync=false

gst-launch-1.0 udpsrc port=5610 caps="application/x-rtp, media=(string)audio, \
 clock-rate=(int)8000, encoding-name=(string)PCMA" ! rtppcmadepay ! \
 audio/x-alaw, rate=8000, channels=1 ! alawdec ! autoaudiosink sync=false