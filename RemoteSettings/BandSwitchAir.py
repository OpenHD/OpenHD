import configparser
import binascii
from itertools import chain
import os
import sys
import fileinput
import socket
import re

from pathlib import Path
import subprocess
from subprocess import call
from tempfile import mkstemp
from shutil import move
from os import fdopen, remove
from time import sleep
import argparse
import threading
lock = threading.Lock()


parser = argparse.ArgumentParser()
parser.add_argument("-DefaultBandWidthAth9k", type=int, help="")



args = parser.parse_args()
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




InMsgBand5 = bytearray(b'5') #Air to 5
InMsgBand10 = bytearray(b'a') #Air to 10
InMsgBand20 = bytearray(b'0') #Air to 20


PrimaryCardPath = "Non"
IsAth9kFound = False

def SendDataToGround(MessageBuf):
    UDP_PORT = 8943
    sockToAir = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
    sockToAir.sendto( bytes(MessageBuf,'utf-8'), ('127.0.0.1', UDP_PORT))

def SwitchLocalBandTo(PathToFile, Mode):
    Buff = ""
    if Mode == 5:
        Buff = "0x00000005"
    elif Mode == 10:
        Buff = "0x0000000a"
    elif Mode == 20:
        Buff = "0x00000000"
    else:
        print("Error: Incorrect band requested \n")
        return False


    try:
        hFile = open(PathToFile, 'w')
        hFile.write(Buff)
        hFile.close()
    except Exception as e:
        print("Error: thrown exception while processing file \n" + PathToFile + " E Message:  "  + str(e))
        return False
    return True


def FindCardPhyInitPath():
    global PrimaryCardPath

    for root, dirs, files in os.walk("/sys/kernel/debug/ieee80211/"):
        for varname in files:
            if varname.__contains__("chanbw") == True:
                PrimaryCardPath = root
                PrimaryCardPath = PrimaryCardPath + "/chanbw"
                print("Primary card path: " + PrimaryCardPath);

    if PrimaryCardPath == "Non":
        print("Failed to init Ath9k card with 5Mhz patch");
        return False

    return True


def AirToGroundRSSI():
    while True:

        lock.acquire()
        band = CurrentBand
        lock.release()
        SendDataToGround(band)
        sleep(0.5)


def NotifyCameraControl(MessageBuf):
    print("CurrentBand: ", CurrentBand )
    #add code that will send message over UDP  4324 to cameracontrolUDP.py
    UDP_PORT = 4324
    sockToAir = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
    sockToAir.sendto( bytes(MessageBuf,'utf-8'), ('127.0.0.1', UDP_PORT))




def StartRecv():
    global CurrentBand
    UDP_IP = ""
    UDP_PORT = 4322 #3033 - UDP DownLink from Ground
    sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    sock.bind((UDP_IP, UDP_PORT))
    while True:
        data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
	#switch band only with ath9k
        if IsAth9kFound == True:
            if data == InMsgBand5 and CurrentBand != "5":
                print("InMsgBand5\n")
                if SwitchLocalBandTo(PrimaryCardPath,5) == True:
                    print("InMsgBand5\n")
                    lock.acquire()
                    CurrentBand = "5"
                    lock.release()
                    NotifyCameraControl(CurrentBand)
                    #os.system('/usr/local/share/RemoteSettings/KillRaspivid.sh')
                    #os.system('/dev/shm/startReadCameraTransfer_5.sh &')

                
            if data == InMsgBand10 and CurrentBand != "a":
                print("InMsgBand10\n")
                if SwitchLocalBandTo(PrimaryCardPath,10) == True:
                    print("InMsgBand10\n")
                    lock.acquire()
                    CurrentBand = "a"
                    lock.release()
                    NotifyCameraControl(CurrentBand)
                    #os.system('/usr/local/share/RemoteSettings/KillRaspivid.sh')
                    #os.system('/dev/shm/startReadCameraTransfer_10.sh &')

                
            if data == InMsgBand20 and CurrentBand != "0":
                print("InMsgBand20\n")
                if SwitchLocalBandTo(PrimaryCardPath,20) == True:
                    print("InMsgBand20\n")
                    lock.acquire()
                    CurrentBand = "0"
                    lock.release()
                    NotifyCameraControl(CurrentBand)
                    #os.system('/usr/local/share/RemoteSettings/KillRaspivid.sh')
                    #os.system('/dev/shm/startReadCameraTransfer.sh &')
                 
        print(".")


IsAth9kFound =  FindCardPhyInitPath()
if IsAth9kFound == False:
    print("Secondary camera switch part still enabled") 
RSSIThread = threading.Thread(target=AirToGroundRSSI)
RSSIThread.start()

StartRecv()