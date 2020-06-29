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
import threading
lock = threading.Lock()


import configparser
import binascii
from itertools import chain
import os
import sys
import fileinput
import socket
import re

import subprocess
from subprocess import call
from tempfile import mkstemp
from shutil import move
from os import fdopen, remove

import random
import string

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

parser.add_argument("-DefaultCameraId", type=int, help="Arducamera board default camera id")

parser.add_argument("-BitrateMeasured", type=int, help="")
parser.add_argument("-SecondaryCamera", help="IP,USB")
parser.add_argument("-CameraType", help="RPi,RPiAndSecondary,Secondary")
parser.add_argument("-WithoutNativeRPiCamera", help="1,0")
parser.add_argument("-DefaultBandWidthAth9k", type=int, help="")

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

BitrateMeasured = args.BitrateMeasured
SecondaryCamera = args.SecondaryCamera
CameraType = args.CameraType
WithoutNativeRPiCamera = args.WithoutNativeRPiCamera

tmp = args.DefaultBandWidthAth9k
CurrentBand = "0"
if tmp == 20:
    CurrentBand="0"
elif tmp == 10:
    CurrentBand="a"
elif tmp == 5:
    CurrentBand="5"
else:
    CurrentBand = "0"

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

InMsgCameraTypeRPi = bytearray(b'RPi')
InMsgCameraTypeRPiAndSecondary = bytearray(b'RPiAndSecondary')
InMsgCameraTypeSecondary = bytearray(b'Secondary')

InMsgBand5 = bytearray(b'5') #Air to 5
InMsgBand10 = bytearray(b'a') #Air to 10
InMsgBand20 = bytearray(b'0') #Air to 20

ChannelValueCurrent=-1
ChannelValueNew=0
SwitchCameraEvent=0

gp.setmode(gp.BCM)

def InitGPIO():
    print("GPIO init start...")
    gp.setup(4, gp.OUT) 
    gp.setup(17, gp.OUT)
    gp.setup(18, gp.OUT)

    gp.setup(22, gp.OUT)
    gp.setup(23, gp.OUT)
    gp.setup(9, gp.OUT)
    gp.setup(25, gp.OUT)
    
    if IsArduCameraV21 == 0:
        gp.output(17, True)
        gp.output(18, True)
        gp.output(22, True)
        gp.output(23, True)
        gp.output(9, True)
        gp.output(25, True)

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

###########################################new start

def SwitchCamera(InCameraType, InBand):
    print("CameraType: ", InCameraType)
    print("CurrentBand: ", InBand )
    if InCameraType == "RPi":
        try:
            os.system('/usr/local/share/RemoteSettings/KillIPCamera.sh 2>/dev/null')
            os.system('/usr/local/share/RemoteSettings/KillUSBCamera.sh 2>/dev/null')
            os.system('/usr/local/share/RemoteSettings/KillRaspivid.sh 2>/dev/null')
            if InBand == "0":
                os.system('/dev/shm/startReadCameraTransfer.sh &')
            if InBand == "a":
                os.system('/dev/shm/startReadCameraTransfer_10.sh &')
            if InBand == "5":
                os.system('/dev/shm/startReadCameraTransfer_5.sh &')
        except Exception as e:
            print(e)

    if InCameraType == "RPiAndSecondary":
        try:
            BitrateSecondaryCamera = 1000000
            BitrateMainCamera = 1000000
            if InBand == "0":
                BitrateSecondaryCamera = int(BitrateMeasured / 2)
                BitrateMainCamera = int(BitrateMeasured / 2)
            if InBand == "a":
                BitrateSecondaryCamera = int(BitrateMeasured / 4)
                BitrateMainCamera = int(BitrateMeasured / 4)
            if InBand == "5":
                BitrateSecondaryCamera = int(BitrateMeasured / 8)
                BitrateMainCamera = int(BitrateMeasured / 8)


            print("BitrateMainCamera: ",  BitrateMainCamera )
            print("BitrateUSBCamera: ", BitrateSecondaryCamera )
            os.system('/usr/local/share/RemoteSettings/KillIPCamera.sh 2>/dev/null')
            os.system('/usr/local/share/RemoteSettings/KillUSBCamera.sh 2>/dev/null')
            os.system('/usr/local/share/RemoteSettings/KillRaspivid.sh 2>/dev/null')
            subprocess.Popen( ['/dev/shm/startReadCameraTransferExteranlBitrate.sh', str(BitrateMainCamera) ] )

            if SecondaryCamera == "IP":
                os.system('/usr/local/share/RemoteSettings/KillIPCamera.sh 2>/dev/null')
                os.system('/dev/shm/startReadIPCameraLowRes.sh &')
            if SecondaryCamera == "USB":
                os.system('/usr/local/share/RemoteSettings/KillUSBCamera.sh 2>/dev/null')     
                subprocess.Popen( ['/dev/shm/startReadUSBCamera.sh', str(BitrateSecondaryCamera) ] )
        except Exception as e:
            print(e)
    if InCameraType == "Secondary":
        try:
            BitrateSecondaryCamera = 1000000
            if InBand == "0":
                BitrateSecondaryCamera = int(BitrateMeasured) 
            if InBand == "a":
                BitrateSecondaryCamera = int(BitrateMeasured / 2)
            if InBand == "5":
                BitrateSecondaryCamera = int(BitrateMeasured / 4)

            os.system('/usr/local/share/RemoteSettings/KillIPCamera.sh  2>/dev/null')
            os.system('/usr/local/share/RemoteSettings/KillUSBCamera.sh 2>/dev/null')
            os.system('/usr/local/share/RemoteSettings/KillRaspivid.sh  2>/dev/null')

            if SecondaryCamera == "IP":
                os.system('/usr/local/share/RemoteSettings/KillRaspivid.sh 2>/dev/null')
                os.system('/usr/local/share/RemoteSettings/KillIPCamera.sh  2>/dev/null')
                if InBand == "0":
                    os.system('/dev/shm/startReadIPCameraHiRes.sh &')
                if InBand == "a":
                    os.system('/dev/shm/startReadIPCameraLowRes.sh &')
                if InBand == "5":
                    os.system('/dev/shm/startReadIPCameraLowRes.sh &')
            if SecondaryCamera == "USB":
                os.system('/usr/local/share/RemoteSettings/KillRaspivid.sh 2>/dev/nulll')
                os.system('/usr/local/share/RemoteSettings/KillUSBCamera.sh 2>/dev/null')
                subprocess.Popen( ['/dev/shm/startReadUSBCamera.sh', str(BitrateSecondaryCamera) ] )
        except Exception as e:
            print(e)

###########################################new end

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
    #shellcommand = "/dev/shm/startReadCameraTransfer.sh &"
    #print("Shell Command to start transfer video: ", shellcommand)
    #os.system(shellcommand)
    SwitchCamera(CameraType, CurrentBand)
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
            gp.output(4,  False)
            gp.output(17,  False)
            gp.output(18,  True)
            ActiveCameraId = id
        if id == 2:
            print("SW GPIO 2")
            if IsArduCameraV21 == 21:
                i2c = "i2cset -y 1 0x70 0x00 0x05"
                os.system(i2c)
            gp.output(4,  True)
            gp.output(17,  False)
            gp.output(18,  True)
            ActiveCameraId = id
        if id == 3:
            print("SW GPIO 3")
            if IsArduCameraV21 == 21:
                i2c = "i2cset -y 1 0x70 0x00 0x06"
                os.system(i2c)
            gp.output(4,  False)
            gp.output(17,  True)
            gp.output(18,  False)
            ActiveCameraId = id
        if id == 4:
            if IsArduCameraV21 == 21:
                i2c = "i2cset -y 1 0x70 0x00 0x07"
                os.system(i2c)
            gp.output(4, True)
            gp.output(17, True)
            gp.output(18, False)
            ActiveCameraId = id
        print("GPIO Switched to: ", id)
    else:
        print("new ID eq Active. GPIO state unchanged.")
    print("New id == ", ActiveCameraId)
    
    

     
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

def SwitchThread():
    global SwitchCameraEvent
    while True:
        lock.acquire()
        IsTime=SwitchCameraEvent
        SwitchCameraEvent=0
        lock.release()
        if IsTime == 1:
            lock.acquire()
            Band=CurrentBand
            Type=CameraType
            lock.release()
            SwitchCamera(Type, Band)
        sleep(0.5)

            
def SendDataToGround(MessageBuf):
    UDP_PORT = 8943
    sockToAir = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
    sockToAir.sendto(MessageBuf, ('127.0.0.1', UDP_PORT))

def random_string(length):
    pool = string.letters + string.digits
    return ''.join(random.choice(pool) for i in xrange(length))

def AirToGroundNotifyCameraModeThread():
    while True:
        SendDataToGround(SessionID)
        lock.acquire()
        type = CameraType
        lock.release()
        SendDataToGround(type)
        sleep(0.2)

SocketBand=0
def StartRecvBandMsg():
    global CurrentBand
    global SwitchCameraEvent
    global SocketBand
    UDP_IP = ""
    UDP_PORT = 4324
    try:
        SocketBand = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
        SocketBand.bind((UDP_IP, UDP_PORT))
    except Exception as e:
        print("Except: Bind to port 4324 " + str(e) ) 
    while True:
        data, addr = SocketBand.recvfrom(1024) # buffer size is 1024 bytes
        if data == InMsgBand5:
            lock.acquire()
            CurrentBand = "5"
            SwitchCameraEvent = 1
            lock.release()
        if data == InMsgBand10:
            lock.acquire()
            CurrentBand = "a"
            SwitchCameraEvent = 1
            lock.release()
        if data == InMsgBand20:
            lock.acquire()
            CurrentBand = "0"
            SwitchCameraEvent = 1
            lock.release()


SocketCameraType = 0
def StartRecvCameraTypeMsg():
    global CameraType
    global SwitchCameraEvent
    global SocketCameraType
    UDP_IP = ""
    UDP_PORT = 4323
    try:
        SocketCameraType = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
        SocketCameraType.bind((UDP_IP, UDP_PORT))
    except Exception as e:
        print("Except: Bind to port 4323: " + str(e) ) 
    while True:
        data, addr = SocketCameraType.recvfrom(1024) # buffer size is 1024 bytes
        if data == InMsgCameraTypeRPi and CameraType != "RPi" and WithoutNativeRPiCamera != "1" :
            lock.acquire()
            CameraType = "RPi"
            SwitchCameraEvent = 1
            lock.release()
        if data == InMsgCameraTypeRPiAndSecondary and CameraType != "RPiAndSecondary" and WithoutNativeRPiCamera != "1" :
            lock.acquire()
            CameraType = "RPiAndSecondary"
            SwitchCameraEvent = 1
            lock.release()
        if data == InMsgCameraTypeSecondary and CameraType != "Secondary":
   
            lock.acquire()
            CameraType = "Secondary"
            SwitchCameraEvent = 1
            lock.release()


InBandThread = threading.Thread(target=StartRecvBandMsg)
InBandThread.daemon = True
InBandThread.start()

InCameraTypeThread = threading.Thread(target=StartRecvCameraTypeMsg)
InCameraTypeThread.daemon = True
InCameraTypeThread.start()

SwitchCheckThread = threading.Thread(target=SwitchThread)
SwitchCheckThread.daemon = True
SwitchCheckThread.start()



IDTemp=random_string(32)
SessionID="SessionID" + IDTemp
NotifyThread = threading.Thread(target=AirToGroundNotifyCameraModeThread)
NotifyThread.daemon = True
NotifyThread.start()




#Arducamera Board:       
while True:

    d = s.recvfrom(2)
    data = d[0]
    chValue = struct.unpack("H", data)[0]
    
    ChannelValueNew = chValue

    if ChannelValueNew != ChannelValueCurrent:
        if ChannelValueNew >= Camera1ValueMin and ChannelValueNew <= Camera1ValueMax and IsCamera1Enabled == 1:
            ChangeTo(1)
        if ChannelValueNew >= Camera2ValueMin and ChannelValueNew <= Camera2ValueMax  and IsCamera2Enabled == 1:
            ChangeTo(2)
        if ChannelValueNew >= Camera3ValueMin and ChannelValueNew <= Camera3ValueMax and IsCamera3Enabled == 1:
            ChangeTo(3)
        if ChannelValueNew >= Camera4ValueMin and ChannelValueNew <= Camera4ValueMax and IsCamera4Enabled == 1:
            ChangeTo(4)
    
    ChannelValueCurrent = ChannelValueNew


def signal_handler(signal, frame):
    print('Exit code...')
    StopCamera()
    try:
        gp.cleanup()
        
        InBandThread._stop()
        InCameraTypeThread._stop()
        SwitchCheckThread._stop()
        NotifyThread._stop()
        shutdown(s, SHUT_RDWR)
        shutdown(SocketCameraType, SHUT_RDWR)
        shutdown(SocketBand, SHUT_RDWR)
        s.close()
        SocketCameraType.close()
        SocketBand.close()


        print("Cleaning process done.")
    except:
        print("except in signal_handler")
        print("Completed")
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)
