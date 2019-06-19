ps -ef | nice grep "gst-launch-1.0 uvch264src device=/dev/vid" | nice grep -v grep | awk '{print $2}' | xargs kill -9
ps -ef | nice grep "gst-launch-1.0 v4l2src device=/dev/vid" | nice grep -v grep | awk '{print $2}' | xargs kill -9
ps -ef | nice grep "gst-launch-1.0 videotestsrc" | nice grep -v grep | awk '{print $2}' | xargs kill -9
