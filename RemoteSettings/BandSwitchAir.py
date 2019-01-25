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


def StartRecv():
    global CurrentBand
    UDP_IP = ""
    UDP_PORT = 4321 #3033 - UDP DownLink from Ground
    sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    sock.bind((UDP_IP, UDP_PORT))
    while True:
        data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
        if data == InMsgBand5:
            print("InMsgBand5\n")
            if SwitchLocalBandTo(PrimaryCardPath,5) == True:
                os.system('/home/pi/RemoteSettings/KillRaspivid.sh')
                os.system('/dev/shm/startReadCameraTransfer_5.sh &')
            lock.acquire()
            CurrentBand = "5"
            lock.release()
        if data == InMsgBand10:
            print("InMsgBand10\n")
            if SwitchLocalBandTo(PrimaryCardPath,10) == True:
                os.system('/home/pi/RemoteSettings/KillRaspivid.sh')
                os.system('/dev/shm/startReadCameraTransfer_10.sh &')
            lock.acquire()
            CurrentBand = "a"
            lock.release()
        if data == InMsgBand20:
            print("InMsgBand20\n")
            if SwitchLocalBandTo(PrimaryCardPath,20) == True:
                os.system('/home/pi/RemoteSettings/KillRaspivid.sh')
                os.system('/dev/shm/startReadCameraTransfer.sh &')
            lock.acquire()
            CurrentBand = "0"
            lock.release()


if FindCardPhyInitPath() == True:
    RSSIThread = threading.Thread(target=AirToGroundRSSI)
    RSSIThread.start()

    StartRecv()

else:
    print("Flase")
