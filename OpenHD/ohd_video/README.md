## Summary

This submodule is responsible for detecting the cameras connected to the system and then starts an encoded video stream for each of the discovered cameras.
In short,the functionalities it exposes to the (main) openhd executable are:
1) Detect connected camera(s)
2) Setup a pipeline that generates a continuous stream of encoded video data (h264,h265 or mjpeg) for a detected camera
   This data is forwarded via a callback (previously udp port)
3) Store and change camera/encoder-related settings

Camera settings are stored in /usr/local/share/openhd/video
The Camera manifest (list of discovered cameras) can be found under /tmp for debugging.


##Note 
The code in this module must adhere to the following paradigms:
1) It only generates encoded video data,then forwards it. It doesn't know if the video data is actually picked up or makes it to the
   ground.
2) It never runs on the ground pi, only on the air pi.
3) There are no code dependencies to other modules like ohd_interface.
4) to adhere with 1), for h264/h265 streaming, re-send the "configuration data" (aka SPS,PPS,key frame for h264, SPS,PPS,VPS,key frame for h265)
in regular intervals. This way the decoding application can start the video decoding after max. 1 interval size, assuming a connection
without packet drops