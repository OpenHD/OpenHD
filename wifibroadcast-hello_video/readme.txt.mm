Wifibroadcast real-time video viewer for Raspberry Pi3


The program is a modified version of the Hello_video demo. The goal is stutter-free video.  

[WifiBroadcast Rx] -> [Linux stdout -> stdin Fifo] -> Hello_videoxx.bin -> [60FPS hdmi monitor] 

The program is in pre-beta status.
It's made exclusively for 60FPS hdmi monitors, so the Linux output must set to 60FPS.

There are 2 versions, choose according to the Tx fps setting:

Hello_video_48.bin
It is tested with Rodizio default settings and:
- 48FPS Tx (v1 camera) and 60FPS hdmi monitor
  Latency is about 140msec.
- 59.9FPS Tx (v2 camera) and 60FPS hdmi monitor
(The 59.9 is an ugly workaround, because the program on 60FPS input doesn't handle yet the latency drift, so the slower Tx setting solves the possible crystal inaccuracy drift. Next version will handle that.)
 

Hello_video_30.bin
It is tested with:
30FPS Tx (v1 camera) and 60FPS hdmi monitor
Latency is 6 frames, 200msec. In the first few seconds maybe more, but decreases to 6 frames in 3-5 seconds.

mmormota
