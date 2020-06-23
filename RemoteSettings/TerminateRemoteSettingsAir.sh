#!/bin/bash

ps -ef | nice grep "wfb_rx -c 127.0.0.1 -u 9393 -p 90" | nice grep -v grep | awk '{print $2}' | xargs kill -9
ps -ef | nice grep "wfb_tx -u 9292 -p 91 -B 20 -M 0" | nice grep -v grep | awk '{print $2}' | xargs kill -9
