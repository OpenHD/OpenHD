#!/usr/bin/python3

import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)
GPIO.setup(26, GPIO.IN, pull_up_down=GPIO.PUD_UP)

IsEnabled = GPIO.input(26)
if(IsEnabled == True):
    print("NotPressed")
else:
    print("Pressed")
