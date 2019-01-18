#!/bin/bash
#
# Common settings (need to be kept in sync for both TX and RX!)
# ============================================================
#
# Desired frequency in MHz
FREQ=2472
#
# Set to "Y" on the RX for auto-scanning. Frequency still has to be set on TX!
# Feature might be buggy or not work at all!
FREQSCAN=N
#
# the following frequencies are supported:
# 2412, 2417, 2422, 2427, 2432, 2437, 2442, 2447, 2452, 2457, 2462, 2467, 2472, 2484 (Ralink and Atheros)
# 2312, 2317, 2322, 2327, 2332, 2337, 2342, 2347, 2352, 2357, 2362, 2367, 2372, 2377, 2382, 2387, 2392, 2397, 2402, 2407 (Atheros only)
# 2477, 2482, 2487, 2489, 2492, 2494, 2497, 2499, 2512, 2532, 2572, 2592, 2612, 2632, 2652, 2672, 2692, 2712 (Atheros only)
#
# 5180, 5200, 5220, 5240, 5260, 5280, 5300, 5320
# 5500, 5520, 5540, 5560, 5580, 5600, 5620, 5640, 5660, 5680, 5700
# 5745, 5765, 5785, 5805, 5825
#
# 2.3Ghz and 2.5-2.7Ghz band only works with Atheros cards. Check your local regulations before transmitting!
# Frequencies higher than 2512MHz work with Atheros, but with a lot lower transmit power and sensitivity and
# thus greatly reduced range. Only useable for short-range applications!
#
#
# Set this to "single" for single TX wifi card, for dual TX wifi cards set "dual".
# MAC addresses and frequency for the RX and TX wifi need to be set here when dual TX mode is enabled.
#
TXMODE=single
#
MAC_RX[0]=00c0ca91ee30
FREQ_RX[0]=2484
#
MAC_RX[1]=24050f953384
FREQ_RX[1]=2484
#
MAC_RX[2]=24050f953378
FREQ_RX[2]=2484
#
MAC_RX[3]=24050f953373
FREQ_RX[3]=2484
#
#
MAC_TX[0]=24050f953378
FREQ_TX[0]=5745
#
MAC_TX[1]=ec086b1c7834
FREQ_TX[1]=2472
#
#
# Wifi Datarate. Lower settings yield higher range and vice versa.
# 1=5.5Mbit, 2=11Mbit, 3=12Mbit, 4=19.5Mbit/18Mbit, 5=24Mbit, 6=36Mbit
DATARATE=4
#
#
# Choose between 30, 40, 48, 59.9
FPS=48
#
#
# FEC SETTINGS
# max. blocklength Ralink = 2278, Atheros = 1550
# min. sensible blocklength ~ 700
VIDEO_BLOCKS=8
VIDEO_FECS=4
VIDEO_BLOCKLENGTH=1024
#
#
# Telemetry transmission method:
# wbc = use wifibroadcast as telemetry up/downlink
# external = use external means as telemetry up/downlink (LRS or 3DR dongles)
# if set to external, set serialport to which LRS or 3DR dongle is connected
# both on ground and air pi
TELEMETRY_TRANSMISSION=wbc
#
#
# Set to "disabled" or "mavlink" for Mavlink (Tower App, Missionplanner, etc.)
TELEMETRY_UPLINK=mavlink
#
#
# Set this to "mavlink" to enable R/C over wifibroadcast using mavlink protocol or "msp" for MSP protocol
# Set to "sumd" for Graupner SUMD, "ibus" for Flysky IBUS, "srxl" for Multiplex SRXL / XBUS Mode B. Set to "disabled" to disable
# See joyconfig.txt for other settings, default settings work for Taranis in USB Joystick mode
RC=disabled
#
#
#
# TX settings
# ============================================================
#
# set to "auto" for automatic video bitrate measuring. Set to a fixed value to
# disable automatic measuring
VIDEO_BITRATE=auto
#
#
# if VIDEO_BITRATE above is set to "auto" the videobitrate will be determined
# by measuring the available bitrate and multiplying it with BITRATE_PERCENT
# Depending on channel utilization by other wifi networks you may need to set
# this to a lower value like 60% to avoid a delayed video stream.
# On free channels you may set this to a higher value like 75% to get a higher
# bitrate and thus image quality.
BITRATE_PERCENT=65
#
#
# CTS protection (use in areas with other wifi networks)
# set to "auto" for automatic detection, "N" to disable, "Y" to enable
CTS_PROTECTION=auto
#
#
# Camera image settings
# V1 cam: 1280x720: 30fps, 48fps. 1920x1080: 30fps
# V2 cam: 1280x720: 30fps, 48fps, 59.9fps. 1640x922: 30fps, 40fps. 1920x1080: 30fps
WIDTH=1280
HEIGHT=720
#
#
# Lower values mean faster glitch-recovery, but also lower video quality.
# With fps=48 and keyframerate=5, glitches will stay visible for around 100ms in worst case.
# You can set this higher or lower according to your needs. Minimum value is 2.
KEYFRAMERATE=5
#
#
# Set additional raspivid parameters here
EXTRAPARAMS="-cd H264 -n -fl -ih -pf high -if both -ex sports -mm average -awb horizon"
#
#
# Serial port and baudrate (19200 is minimum) to use for the R/C connection between air Pi and flight control
# Set this to "/dev/serial0" for Pi onboard serial port or  "/dev/ttyUSB0" for USB-to-serial adapter
FC_RC_SERIALPORT=/dev/serial0
FC_RC_BAUDRATE=57600
#
#
# Serial port and baudrate to use for the telemetry connection between air Pi and flight control
# Set this to "/dev/serial0" for Pi onboard serial port or  "/dev/ttyUSB0" for USB-to-serial adapter
FC_TELEMETRY_SERIALPORT=/dev/serial0
FC_TELEMETRY_BAUDRATE=57600
#
#
# not supported yet, do not change
FC_MSP_SERIALPORT=/dev/ttyUSB0
FC_MSP_BAUDRATE=115200
#
#
#
# RX settings
# ============================================================
#
# Set to "Y" to scan for wifi networks with airodump-ng before starting RX
AIRODUMP=N
#
# Number of seconds wifi scanner is shown. Minimum recommended scanning time is 25 seconds.
AIRODUMP_SECONDS=25
#
#
# set this to "Y" to enable Wifi Hotspot. Default SSID is "EZ-Wifibroadcast", password is "wifibroadcast".
# See apconfig.txt for configuration. This will forward the received video and telemetry streams to a
# smartphone/tablet or computer connected to the RX Pi via WiFi.
WIFI_HOTSPOT=N
#
# Set to "internal" to use the interal Pi3 wifi chip or the MAC address of the USB wifi card you want to use
WIFI_HOTSPOT_NIC=internal
#
# set this to "Y" to enable Ethernet hotspot. This will forward the received video and telemetry streams
# to another computer or other device connected to the Raspberry via Ethernet
ETHERNET_HOTSPOT=N
#
# Set to "Y" to enable periodic screenshots every 10 seconds
ENABLE_SCREENSHOTS=N
#
# Set to "memory" to use RAMdisk for temporary video/screenshot/telemetry storage. This limits recording time
# to ~12-14 minutes, but is the safe way. If you need longer recording times, use "sdcard", to use the sdcard
# as the temporary video storage. Keep in mind though, that this might introduce video stutter and/or bad blocks.
# Use a fast sdcard and TEST CAREFULLY BEFORE USING!
VIDEO_TMP=memory
#
#
# set this to "Y" to enable wifibroadcast relay mode. This will forward the received video and telemetry streams
# to another wifibroadcast RX. Note! Currently, the RSSI display you see on the RX behind the relay is not the RSSI
# between aircraft and ground, but between relay and rx on the ground!
RELAY=N
RELAY_NIC=112233445566
RELAY_FREQ=5220
#
# Set this to "Y" to disable text messages about Display and Wifi card setup etc.
QUIET=N
#
#
# serial port settings if using TELEMETRY_TRANSMISSION=external
EXTERNAL_TELEMETRY_SERIALPORT_GROUND=/dev/serial0
EXTERNAL_TELEMETRY_SERIALPORT_GROUND_BAUDRATE=57600
#
#
# set to "Y" to enable output of telemetry to serialport on ground Pi (for antenna tracker etc.)
ENABLE_SERIAL_TELEMETRY_OUTPUT=N
# baudrate and serialport used for ground Pi telemetry output
TELEMETRY_OUTPUT_SERIALPORT_GROUND=/dev/serial0
TELEMETRY_OUTPUT_SERIALPORT_GROUND_BAUDRATE=57600
#
#
# Set this to "raw" to forward a raw h264 stream to 2nd display devices (for FPV_VR app)
# Set to "rtp" to forward RTP h264 stream (for Tower app and gstreamer etc.)
FORWARD_STREAM=rtp
#
# UDP port to send video stream to, set to 5000 for FPV_VR app or 5600 for Mission Planner
VIDEO_UDP_PORT=5600
#
#
# Mavlink forwarder to use. Choose "mavlink-routerd" or "cmavnode"
MAVLINK_FORWARDER=mavlink-routerd
#
#
# Set this to "Y" to enable collection of extra debug logs. If you experience any issues,
# please reproduce them with debug set to "Y" and plug a USB memory stick afterwards, you
# will find the debug logs on the memory stick.
DEBUG=N
#
