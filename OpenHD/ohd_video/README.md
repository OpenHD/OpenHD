## FOR NOW:

Relies on two unidirectional links (direction: air pi to ground pi) setup by openhd-interface.
(One link for primary video down, one link for secondary video down).

This module uses the generated .json by openhd-system to start a camera stream for each of the discovered cameras.
Resolution and format as well as other params are fixed for now, we'l add settings via mavlink here.

Note that code in this module should follow the following paradigm:

1) It only generates encoded video data, without knowing if the video data is actually picked up or makes it to the
   ground.
2) It never runs on the ground pi, only on the air pi.
3) There are no code dependencies to other modules like ohd_interface.

BIG TODO: Introduce settings in a sensible manner.