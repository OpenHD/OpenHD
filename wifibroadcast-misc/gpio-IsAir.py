#!/usr/bin/python

import RPi.GPIO as GPIO
import os

GPIO.setmode(GPIO.BCM)
GPIO.setup(7, GPIO.IN, pull_up_down=GPIO.PUD_UP)

input_state0 = GPIO.input(7)

# True = not connected, False = connected to GND
IsFileExists = os.path.isfile('/boot/air.txt')
if (input_state0 == False or IsFileExists == True):
    print ('Boot as RPi Air')
    f = open("/tmp/Air", "w")
    f.write("1")
    f.close()
    quit()
