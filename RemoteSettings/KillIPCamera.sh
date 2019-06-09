ps -ef | nice grep "gst-launch-1.0 rtspsrc location=" | nice grep -v grep | awk '{print $2}' | xargs kill -9
