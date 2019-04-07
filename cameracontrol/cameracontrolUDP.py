#!/usr/bin/env python
# -*- coding: utf-8 -*-


from __future__ import print_function

from time import sleep
import RPi.GPIO as gp
import os
import signal
import sys
import psutil
import argparse
import socket
import struct


parser = argparse.ArgumentParser()
parser.add_argument("-IsArduCameraV21", type=int, help="")
parser.add_argument("-IsCamera1Enabled", type=int, help="Enable or Disable Multi Camera Adapter Camera Port 1")
parser.add_argument("-IsCamera2Enabled", type=int, help="Enable or Disable Multi Camera Adapter Camera Port 2")
parser.add_argument("-IsCamera3Enabled", type=int, help="Enable or Disable Multi Camera Adapter Camera Port 3")
parser.add_argument("-IsCamera4Enabled", type=int, help="Enable or Disable Multi Camera Adapter Camera Port 4")

parser.add_argument("-Camera1ValueMin", type=int, help="")
parser.add_argument("-Camera1ValueMax", type=int, help="")

parser.add_argument("-Camera2ValueMin", type=int, help="")
parser.add_argument("-Camera2ValueMax", type=int, help="")

parser.add_argument("-Camera3ValueMin", type=int, help="")
parser.add_argument("-Camera3ValueMax", type=int, help="")

parser.add_argument("-Camera4ValueMin", type=int, help="")
parser.add_argument("-Camera4ValueMax", type=int, help="")

parser.add_argument("-DefaultCameraId", type=int, help="")
args = parser.parse_args()

############################# Assigning values ...
IsArduCameraV21 = args.IsArduCameraV21
IsCamera1Enabled = args.IsCamera1Enabled
IsCamera2Enabled = args.IsCamera2Enabled
IsCamera3Enabled = args.IsCamera3Enabled
IsCamera4Enabled = args.IsCamera4Enabled

DefaultCameraId = args.DefaultCameraId

Camera1ValueMin = args.Camera1ValueMin
Camera1ValueMax = args.Camera1ValueMax

Camera2ValueMin = args.Camera2ValueMin
Camera2ValueMax = args.Camera2ValueMax

Camera3ValueMin = args.Camera3ValueMin
Camera3ValueMax = args.Camera3ValueMax

Camera4ValueMin = args.Camera4ValueMin
Camera4ValueMax = args.Camera4ValueMax
############################ Print values For debug.
print("IsArduCameraV21: ", IsArduCameraV21)
print("IsCamera1Enable: ", IsCamera1Enabled)
print("IsCamera2Enable: ", IsCamera2Enabled)
print("IsCamera3Enable: ", IsCamera3Enabled)
print("IsCamera4Enable: ", IsCamera4Enabled)

print("DefaultCameraId: ",DefaultCameraId  )

print("Camera1ValueMin: ",Camera1ValueMin )
print("Camera1ValueMax: ",Camera1ValueMax )
print("Camera2ValueMin: ",Camera2ValueMin  )
print("Camera2ValueMax: ",Camera2ValueMax )
print("Camera3ValueMin: ",Camera3ValueMin )
print("Camera3ValueMax: ",Camera3ValueMax )
print("Camera4ValueMin: ",Camera4ValueMin)
print("Camera4ValueMax: ",Camera4ValueMax )

###############################Static
#DefaultCameraId = 1
#FreqToCheck = 0.4
#ConnectionString = /dev/ttyACM0,115200
#Camera1ValueMin = 1350
#Camera1ValueMax = 1650
#IsCamera1Enabled = 1
#Camera2ValueMin = 700
#Camera2ValueMax = 1100
#IsCamera2Enabled = 1
#Camera3ValueMin = 1900
#Camera3ValueMax = 2100
#IsCamera3Enabled = 1
#Camera4ValueMin = 1900
#Camera4ValueMax = 2100
#IsCamera4Enabled = 1


##############################Global
ChannelValueCurrent=-1
ChannelValueNew=0

gp.setmode(gp.BOARD)

def InitGPIO():
    print("GPIO init start...")
    gp.setup(7, gp.OUT) 
    gp.setup(11, gp.OUT)
    gp.setup(12, gp.OUT)

    gp.setup(15, gp.OUT)
    gp.setup(16, gp.OUT)
    gp.setup(21, gp.OUT)
    gp.setup(22, gp.OUT)
    
    if IsArduCameraV21 == 0:
        gp.output(11, True)
        gp.output(12, True)
        gp.output(15, True)
        gp.output(16, True)
        gp.output(21, True)
        gp.output(22, True)

    print("GPIO init ended.")



def ChangeTo(id):
    print("Start swap process...")
#    StopCamera()
    SwitchMultiCameraTo(id)
#    StartCamera()
    print("Swap process ended.")

def KillProcessTree(PID_In,timeout=3):
    "Tries hard to terminate and ultimately kill all the children of this process."
    def on_terminate(proc):
        print("process {} terminated with exit code {}".format(proc, proc.returncode))
    print("Trying to find children of process PID: ", PID_In)
    
    try:
        procs = psutil.Process(PID_In).children()
        # send SIGTERM
        for p in procs:
            print("Trying to terminate: ", p.pid)
            p.terminate()
        gone, alive = psutil.wait_procs(procs, timeout=timeout, callback=on_terminate)
        if alive:
        # send SIGKILL
            for p in alive:
                print("process {} survived SIGTERM; trying SIGKILL" % p)
                p.kill()
            gone, alive = psutil.wait_procs(alive, timeout=timeout, callback=on_terminate)
            if alive:
            # give up
                for p in alive:
                    print("process {} survived SIGKILL; giving up" % p)
    except:
        print("Kill process exception.")


def StopCamera():
    print("stopping camera... ")
    try:
        fpid = open('/dev/shm/TXCAMPID','r')
        ScriptRootPidStr = fpid.readline()
        fpid.close();

        ScriptRootPid = int(ScriptRootPidStr)
        if ScriptRootPid != 0:
            print("Script root PID: ", ScriptRootPid)
            KillProcessTree(ScriptRootPid,3)
    except:
        print("Stop camera exception.")


def StartCamera():
    print("Starting camera...")
    shellcommand = "/dev/shm/startReadCameraTransfer.sh &"
    print("Shell Command to start transfer video: ", shellcommand)
    os.system(shellcommand)
    sleep(0.1)

ActiveCameraId = 0
def SwitchMultiCameraTo(id):
    global ActiveCameraId
    print("ActiveCameraId: ",ActiveCameraId)
    if ActiveCameraId != id:
        if id == 1:
            print("SW GPIO 1")
            if IsArduCameraV21 == 21:
                i2c = "i2cset -y 1 0x70 0x00 0x04"
                os.system(i2c)
            gp.output(7,  False)
            gp.output(11,  False)
            gp.output(12,  True)
            ActiveCameraId = id
        if id == 2:
            print("SW GPIO 2")
            if IsArduCameraV21 == 21:
                i2c = "i2cset -y 1 0x70 0x00 0x05"
                os.system(i2c)
            gp.output(7,  True)
            gp.output(11,  False)
            gp.output(12,  True)
            ActiveCameraId = id
        if id == 3:
            print("SW GPIO 3")
            if IsArduCameraV21 == 21:
                i2c = "i2cset -y 1 0x70 0x00 0x06"
                os.system(i2c)
            gp.output(7,  False)
            gp.output(11,  True)
            gp.output(12,  False)
            ActiveCameraId = id
        if id == 4:
            if IsArduCameraV21 == 21:
                i2c = "i2cset -y 1 0x70 0x00 0x07"
                os.system(i2c)
            gp.output(7, True)
            gp.output(11, True)
            gp.output(12, False)
            ActiveCameraId = id
        print("GPIO Switched to: ", id)
    else:
        print("new ID eq Active. GPIO state unchanged.")
    print("New id == ", ActiveCameraId)
    
    
def signal_handler(signal, frame):
    print('Exit code...')
    StopCamera()
    try:
        gp.cleanup()
        s.close()
        print("Cleaning process done.")
    except:
        print("except in signal_handler")
        print("Completed")
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

     
# Connect to the fly controller and init camera
if IsCamera1Enabled == 1 or IsCamera2Enabled == 1 or IsCamera3Enabled == 1 or IsCamera4Enabled == 1:
    try:

        HOST = ''   # Symbolic name meaning all available interfaces
        PORT = 1257 # Arbitrary non-privileged port
        # Datagram (udp) socket
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        print("Socket created")
        #Bind socket to local host and port
        s.bind((HOST, PORT))
        print("Socket bind complete")
        print("Selected external camera: ", DefaultCameraId)
        InitGPIO()
        SwitchMultiCameraTo(DefaultCameraId)
        StartCamera()
    except:
        print("Open socket exception.")
        InitGPIO()
        SwitchMultiCameraTo(DefaultCameraId)
        print("Result: No input for switch. Action: Init GPIO for camera.  Start camera")
        StartCamera()
        while True:
            sleep(60)
else:
    print("No Multi Camera Adapter Module port selected. Start Camera without GPIO init.")
    StartCamera()
    while True:
        sleep(60)
        
while True:

    d = s.recvfrom(2)
    data = d[0]
    chValue = struct.unpack("H", data)[0]
    
    print(" ")
    print(" Ch value: ", chValue)

    ChannelValueNew = chValue
    print("ChannelValueNew", ChannelValueNew)    

    if ChannelValueNew != ChannelValueCurrent:
        if ChannelValueNew >= Camera1ValueMin and ChannelValueNew <= Camera1ValueMax and IsCamera1Enabled == 1:
            print("Camera1 selected")
            print("ChannelValueNew: ", ChannelValueNew)
            ChangeTo(1)
        if ChannelValueNew >= Camera2ValueMin and ChannelValueNew <= Camera2ValueMax  and IsCamera2Enabled == 1:
            print("Camera2 selected")
            print("ChannelValueNew: ", ChannelValueNew)
            ChangeTo(2)
        if ChannelValueNew >= Camera3ValueMin and ChannelValueNew <= Camera3ValueMax and IsCamera3Enabled == 1:
            print("Camera3 selected")
            ChangeTo(3)
        if ChannelValueNew >= Camera4ValueMin and ChannelValueNew <= Camera4ValueMax and IsCamera4Enabled == 1:
            print("Camera4 selected")
            ChangeTo(4)
    
    ChannelValueCurrent = ChannelValueNew

