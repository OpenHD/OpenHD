ps -ef | nice grep "gst-launch-1.0 udpsrc port=5612" | nice grep -v grep | awk '{print $2}' | xargs kill -9 > /dev/null 2>/dev/null
ps -ef | nice grep "wfb_rx -u 5612 -p 23 -c 127.0.0.1" | nice grep -v grep | awk '{print $2}' | xargs kill -9 > /dev/null 2>/dev/null
ps -ef | nice grep "hello_video.bin.240-befi" | nice grep -v grep | awk '{print $2}' | xargs kill -9 > /dev/null 2>/dev/null
sleep 0.2
