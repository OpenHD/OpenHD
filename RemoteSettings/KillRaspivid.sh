#!/bin/bash


ps -ef | nice grep "raspivid" | nice grep -v grep | awk '{print $2}' | xargs kill -9
ps -ef | nice grep "tx_rawsock -p 0 -b" | nice grep -v grep | awk '{print $2}' | xargs kill -9
