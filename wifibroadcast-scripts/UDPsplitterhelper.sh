#!/bin/bash

while true; do
    /usr/local/bin/udpsplitter.py $1 $2 $3
    sleep 1
done
