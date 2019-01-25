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

InMsgBand5 = bytearray(b'5')
InMsgBand10 = bytearray(b'a')
InMsgBand20 = bytearray(b'0')
AirBand = ""
PrimaryCardPath = "Non"
SlaveCardPath = "Non"
PrimaryCardMAC = "Non"
SlaveCardMAC = "Non"
RC_Value = 0

parser = argparse.ArgumentParser()
parser.add_argument("-PrimaryCardMAC", help="")
parser.add_argument("-SlaveCardMAC", help="")
parser.add_argument("-Band5Below", type=int, help="")
parser.add_argument("-Band10ValueMin", type=int, help="")
parser.add_argument("-Band10ValueMax", type=int, help="")
parser.add_argument("-Band20After", type=int, help="")
parser.add_argument("-DefaultBandWidthAth9k", type=int, help="")


args = parser.parse_args()

PrimaryCardMAC = args.PrimaryCardMAC
SlaveCardMAC = args.SlaveCardMAC
Band5Below = args.Band5Below
Band10ValueMin = args.Band10ValueMin
Band10ValueMax = args.Band10ValueMax
Band20After = args.Band20After
CurrentBand = args.DefaultBandWidthAth9k


def SendDataToAir(MessageBuf):
    UDP_PORT = 4321
    sockToAir = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
    sockToAir.sendto( bytes(MessageBuf,'utf-8'), ('127.0.0.1', UDP_PORT))

def StartRCThreadIn():
    global RC_Value
    UDP_IP = ""
    UDP_PORT = 1258
    sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    sock.bind((UDP_IP, UDP_PORT))

    while True:
        data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
        byteArr = bytearray([data[1], data[0]])
        lock.acquire()
        RC_Value = int.from_bytes( byteArr, byteorder='big')
        lock.release()

def StartRecvThread():
    global AirBand
    UDP_IP = ""
    UDP_PORT = 8943 #2022 - UDP DownLink from Air
    sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    sock.bind((UDP_IP, UDP_PORT))

    while True:
        data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
        print( "Data: " + str(data) )
        if data == InMsgBand5:
            print("InMsgBand5\n")
            lock.acquire()
            AirBand = "5"
            lock.release()

        if data == InMsgBand10:
            print("InMsgBand10\n")
            lock.acquire()
            AirBand = "a"
            lock.release()

        if data == InMsgBand20:
            print("InMsgBand20\n")
            lock.acquire()
            AirBand = "0"
            lock.release()


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


def SwitchRemoteLocalBandTo(band):
    global AirBand
    SendMsgBuf = ""
    if band == 5:
        SendMsgBuf = "5"
    elif band == 10:
        SendMsgBuf = "a"
    elif band == 20:
        SendMsgBuf = "0"
    else:
        print("Error: Incorrect band requested \n")
        return False

    lock.acquire()
    AirBand = "" #Clean old value
    lock.release()


    #switch secondary WiFI card to requestid band
    if SwitchLocalBandTo(SlaveCardPath, band) == False:
        print("Failed to switch secondary card to requested band. Abort.")
        return False

    #send "band switch" request to Air
    switched = 0
    while switched == 0: #Resend until confirmation be received
        print("AirBand start value: " + AirBand)
        SendDataToAir(SendMsgBuf)
        sleep(0.4) #wait in msg
        lock.acquire()
        if band == 5 and AirBand == "5":
            print("band == 5 and AirBand == 5")
            switched = 1
        if band == 10 and AirBand == "a":
            switched = 1
        if band == 20 and AirBand == "0":
            switched = 1
        lock.release()

    #Switch confirmed. Switch primary card to requested band.
    switched = 0
    while switched == 0:  #In case of error try to switch again
        if SwitchLocalBandTo(PrimaryCardPath, band) == True:
            print("Switch local true")
            switched = 1
        else:
            sleep(0.5)

    print("Done. ")
    return True


def FindCardPhyInitPath():
    global PrimaryCardPath
    global SlaveCardPath

    for root, dirs, files in os.walk("/sys/kernel/debug/ieee80211/"):
        for varname in dirs:
            if varname.__contains__(PrimaryCardMAC) == True:
                PrimaryCardPath = root
                PrimaryCardPath = PrimaryCardPath + "/ath9k_htc/chanbw"
                print("Primary card path: " + PrimaryCardPath);

            if varname.__contains__(SlaveCardMAC) == True:
                SlaveCardPath = root
                SlaveCardPath = SlaveCardPath + "/ath9k_htc/chanbw"
                print("Slave card path: " + SlaveCardPath);


    #Confirm that chanbw file exist
    fSlave = Path(SlaveCardPath)
    if fSlave.exists() == False:
        print(SlaveCardPath + " Not found")
        return False

    fPrimary = Path(PrimaryCardPath)
    if fPrimary.exists() == False:
        print(PrimaryCardPath + " Not found")
        return False;

    return True




if FindCardPhyInitPath() == True:
    print("Ok")
    RecvThread = threading.Thread(target=StartRecvThread)
    RecvThread.start()

    RC_UDP_IN_thread = threading.Thread(target=StartRCThreadIn)
    RC_UDP_IN_thread.start()

    #Add command line in code
    while True:
        if RC_Value >= Band20After and CurrentBand != 20 and RC_Value != 0:
            print("Switching to 20...")
            SwitchRemoteLocalBandTo(20)
            CurrentBand = 20

        if RC_Value < Band10ValueMax and RC_Value > Band10ValueMin and CurrentBand != 10 and RC_Value != 0:
            print("Switching to 10...")
            SwitchRemoteLocalBandTo(10)
            CurrentBand = 10

        if RC_Value <= Band5Below and CurrentBand != 5 and RC_Value != 0:
            print("Switching to 5...")
            SwitchRemoteLocalBandTo(5)
            CurrentBand = 5
        sleep(0.6)

else:
    print("Flase")

