#!/usr/bin/python

import time

import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)
GPIO.setup(20, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setmode(GPIO.BCM)
GPIO.setup(21, GPIO.IN, pull_up_down=GPIO.PUD_UP)

input_state0 = GPIO.input(20)
input_state1 = GPIO.input(21)

# True = not connected, False = connected to GND

if (input_state0 == False) and (input_state1 == False):
	print ('openhd-settings-4.txt')
	quit()

if (input_state0 == False) and (input_state1 == True):
	print ('openhd-settings-3.txt')
	quit()

if (input_state0 == True) and (input_state1 == False):
	print ('openhd-settings-2.txt')
	quit()

if (input_state0 == True) and (input_state1 == True):
	print ('openhd-settings-1.txt')
	quit()
