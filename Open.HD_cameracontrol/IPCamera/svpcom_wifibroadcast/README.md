Wifibroadcast
=============

This is a transmitter and receiver of **UDP packets** using **raw WiFi radio**


Main features:
--------------
 - 1:1 map RTP to IEEE80211 packets for minimum latency (doesn't serialize to byte steam)
 - Smart FEC support (immediately yeild packet to video decoder if FEC pipeline without gaps)
 - [Bidirectional mavlink telemetry](https://github.com/svpcom/wifibroadcast/wiki/Setup-HOWTO). You can use it for mavlink up/down and video down link.
 - Automatic TX diversity (select TX card based on RX RSSI)
 - Stream encryption and authentication ([libsodium](https://download.libsodium.org/doc/))
 - Distributed operation. It can gather data from cards on different hosts. So you don't limited to bandwidth of single USB bus.
 - Aggreagation of mavlink packets. Doesn't send wifi packet for every mavlink packet.
 - Enhanced [OSD](https://github.com/svpcom/wifibroadcast_osd) for Raspberry PI (consume 10% CPU on PI Zero)
   Compatible with any screen resolution. Supports aspect correction for PAL to HD scaling.
   
## FAQ
Q: What is a difference from original wifibroadcast?

A: Original version of wifibroadcast use a byte-stream as input and splits it to packets of fixed size (1024 by default). If radio packet was lost and this is not corrected by FEC you'll got a hole at random (unexpected) place of stream. This is especially bad if data protocol is not resistent to (was not desired for) such random erasures. So i've rewrite it to use UDP as data source and pack one source UDP packet into one radio packet. Radio packets now have variable size depends on payload size. This is reduces a video latency a lot.

Q: What type of data can be transmitted using wifibroadcast?

A: Any UDP with packet size <= 1466. For example x264 inside RTP or Mavlink.

Q: What are transmission guarancies?

A: Wifibrodcast use FEC (forward error correction) which can recover 4 lost packets from 12 packets block with default settings. You can tune it (both TX and RX simultaniuosly!) to fit your needs.

Q: Is only Raspberry PI supported?

A: Wifibroadcast is not tied to any GPU - it operates with UDP packets. But to get RTP stream you need a video encoder (with encode raw data from camera to x264 stream). In my case RPI is only used for video encoding (becase RPI Zero is too slow to do anything else) and all other tasks (including wifibroadcast) are done by other board (NanoPI NEO2).

## Theory
Wifibroadcast puts the wifi cards into monitor mode. This mode allows to send and receive arbitrary packets without association and waiting for ACK packets.
[Analysis of Injection Capabilities and Media Access of IEEE 802.11 Hardware in Monitor Mode](https://github.com/svpcom/wifibroadcast/blob/master/patches/Analysis%20of%20Injection%20Capabilities%20and%20Media%20Access%20of%20IEEE%20802.11%20Hardware%20in%20Monitor%20Mode.pdf)

Sample usage chain:
-------------------
```
Camera -> gstreamer --[RTP stream (UDP)]--> wfb_tx --//--[ RADIO ]--//--> wfb_rx --[RTP stream (UDP)]--> gstreamer --> Display
```

For encode logitech c920 camera:
```
gst-launch-1.0 uvch264src device=/dev/video0 initial-bitrate=6000000 average-bitrate=6000000 iframe-period=1000 name=src auto-start=true \
               src.vidsrc ! queue ! video/x-h264,width=1920,height=1080,framerate=30/1 ! h264parse ! rtph264pay ! udpsink host=localhost port=5600
```

To decode:
```
 gst-launch-1.0 udpsrc port=5600 caps='application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264' \
               ! rtph264depay ! avdec_h264 ! clockoverlay valignment=bottom ! autovideosink fps-update-interval=1000 sync=false
```

HOWTO build:
----------------------
For development (inline build)
```
make
```

For binary distribution RHEL or Fedora
```
make rpm
```

For binary distribution Debian or Ubuntu
```
make deb
```

For binary distribution (tar.gz)
```
make bdist
```

You need to generate encryption keys for gs(ground station) and drone:
```
wfb_keygen
```
Leave them inplace for development build or copy to `/etc` for binary install.
Put `drone.key` to drone and `gs.key` to gs.

Supported WiFi hardware:
------------------------
My primary hardware targets are:
1. Realtek RTL8812au. 802.11ac capable. Easy to buy. [**Requires external patched driver!**](https://github.com/svpcom/rtl8812au)  System was tested with ALPHA AWUS036ACH on both sides in 5GHz mode.
2. Ralink RT28xx family. Cheap, but doesn't produced anymore. System was tested with ALPHA AWUS051NH v2 as TX and array of RT5572 OEM cards as RX in 5GHz mode.

To maximize output power and/or increase bandwidth (in case of one-way transmitting) you need to apply kernel patches from ``patches`` directory. See https://github.com/svpcom/wifibroadcast/wiki/Kernel-patches for details.

Wifibroadcast + PX4 HOWTO:
--------------------------
https://dev.px4.io/en/qgc/video_streaming_wifi_broadcast.html

Wiki:
-----
See https://github.com/svpcom/wifibroadcast/wiki for additional info
