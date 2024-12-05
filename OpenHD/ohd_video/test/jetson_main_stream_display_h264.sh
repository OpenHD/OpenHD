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

gst-launch-1.0 -v udpsrc port=5600 caps = "application/x-rtp, media=(string)video, encoding-name=(string)H264, payload=(int)96" ! rtph264depay ! nvv4l2decoder ! nvegltransform ! nveglglessink -e


#or
#gst-launch-1.0 -v udpsrc port=5600 caps = "application/x-rtp, media=(string)video, encoding-name=(string)H264, payload=(int)96" ! rtph264depay ! nvv4l2decoder ! nvvidconv ! glupload ! glimagesink
