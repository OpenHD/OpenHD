## Quick summary:

This library is responsible for detecting the cameras connected to the system 
and then starts a encoded video stream for each of the discovered cameras.
The functionalities it exposes to the public are simple:
1) Send generated, encoded video stream(s) somewhere to be picked up by ohd-interface
2) Expose a means to change camera / encoding specific settings (called by ohd-interface).


##Note 
The code in this module must adhere to the following paradigms:
1) It only generates encoded video data,then forwards it. It doesn't know if the video data is actually picked up or makes it to the
   ground.
2) It never runs on the ground pi, only on the air pi.
3) There are no code dependencies to other modules like ohd_interface.

## List of TODO's
1) Introduce even more settings & validate settings depending on the detected camera(s)
2) add recording to sd card