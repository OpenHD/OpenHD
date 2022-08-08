# bin/bash
# run this to see if encoded data is coming in via udp
# ( Garbage on the console means data is coming out ;) )
# Use this on the air pi when OpenHD is not running, but test_video or test_dummy_gstreamer is running.
# Use this on the ground pi with OpenHD air and ground running.

nc -ul -p 5600