## Summary

OpenHD core (main.cpp) is a c++ executable that creates a working openhd air
or ground unit. While dependencies and more can be validated / checked at compile time,
there are a few 'funky' ways where openhd interacts with the underlying OS. Here is a quick
list of those things:

1) The correct wifi drivers need to be installed
2) All settings (changeable via mavlink and/or used internally) are stored under
    /usr/local/share/openhd/. This directory needs to be write / readable
3) To change CSI cameras on rpi, /usr/local/bin/ohd_camera_setup.sh is called
4) For all wifi hotspot / client functionalities, network manager needs to be installed.
    The wifibroadcast card(s) are taken from nw at start and given back on proper termination
5) For wifi card discovery, iw needs to be installed
6) For usb camera(s), some v4l2 commands might be called