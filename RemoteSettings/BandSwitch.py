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
SlaveCardList = []
PrimaryCardPath = "Non"
PrimaryCardMAC = "Non"
ExitRecvThread = 0
ExitRCThread = 0
ExitRCThread2 = 0

RC_Value = 0
RC_Value2 = 0
LocalVideoMode=1
RemoteVideoMode=1
InMsgCameraTypeRPi = bytearray(b'RPi')
InMsgCameraTypeRPiAndSecondary = bytearray(b'RPiAndSecondary')
InMsgCameraTypeSecondary = bytearray(b'Secondary')

CurrentBandTmp = 0
CurrentBand = 0
SessionID = "0"
RestartDisplay = 0

parser = argparse.ArgumentParser()
parser.add_argument("-PrimaryCardMAC", help="")
parser.add_argument("-Band5Below", type=int, help="")
parser.add_argument("-Band10ValueMin", type=int, help="")
parser.add_argument("-Band10ValueMax", type=int, help="")
parser.add_argument("-Band20After", type=int, help="")

parser.add_argument("-Camera1ValueMin", type=int, help="")
parser.add_argument("-Camera1ValueMax", type=int, help="")

parser.add_argument("-Camera2ValueMin", type=int, help="")
parser.add_argument("-Camera2ValueMax", type=int, help="")

parser.add_argument("-Camera3ValueMin", type=int, help="")
parser.add_argument("-Camera3ValueMax", type=int, help="")

parser.add_argument("-Camera4ValueMin", type=int, help="")
parser.add_argument("-Camera4ValueMax", type=int, help="")


args = parser.parse_args()

PrimaryCardMAC = args.PrimaryCardMAC
Band5Below = args.Band5Below
Band10ValueMin = args.Band10ValueMin
Band10ValueMax = args.Band10ValueMax
Band20After = args.Band20After

Camera1ValueMin = args.Camera1ValueMin
Camera1ValueMax = args.Camera1ValueMax
Camera2ValueMin = args.Camera2ValueMin
Camera2ValueMax = args.Camera2ValueMax
Camera3ValueMin = args.Camera3ValueMin
Camera3ValueMax = args.Camera3ValueMax
Camera4ValueMin = args.Camera4ValueMin
Camera4ValueMax = args.Camera4ValueMax


print("Camera1ValueMin: ",Camera1ValueMin )
print("Camera1ValueMax: ",Camera1ValueMax )
print("Camera2ValueMin: ",Camera2ValueMin  )
print("Camera2ValueMax: ",Camera2ValueMax )
print("Camera3ValueMin: ",Camera3ValueMin )
print("Camera3ValueMax: ",Camera3ValueMax )
print("Camera4ValueMin: ",Camera4ValueMin)
print("Camera4ValueMax: ",Camera4ValueMax )



def SendDataToAir(MessageBuf):
    UDP_PORT = 4321
    sockToAir = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
    sockToAir.sendto( bytes(MessageBuf,'utf-8'), ('127.0.0.1', UDP_PORT))

def StartRCThreadIn():
    global RC_Value
    global ExitRCThread
    UDP_IP = ""
    UDP_PORT = 1258
    RCSock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    RCSock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    RCSock.settimeout(1)
    RCSock.bind((UDP_IP, UDP_PORT))

    while True:
        try:
            data, addr = RCSock.recvfrom(1024) # buffer size is 1024 bytes
            byteArr = bytearray([data[1], data[0]])
            lock.acquire()
            RC_Value = int.from_bytes( byteArr, byteorder='big')
            lock.release()
        except:
            if ExitRCThread == 1:
                print("RC thread exiting...")
                break

    RCSock.close()
    ExitRCThread = 2

def StartRCThreadIn2():
    global RC_Value2
    global ExitRCThread2
    UDP_IP = ""
    UDP_PORT = 1259
    RCSock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    RCSock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    RCSock.settimeout(1)
    RCSock.bind((UDP_IP, UDP_PORT))

    while True:
        try:
            data, addr = RCSock.recvfrom(1024) # buffer size is 1024 bytes
            byteArr = bytearray([data[1], data[0]])
            lock.acquire()
            RC_Value2 = int.from_bytes( byteArr, byteorder='big')
            lock.release()
        except:
            if ExitRCThread2 == 1:
                print("RC thread2 exiting...")
                break

    RCSock.close()
    ExitRCThread2 = 2
   

def StartRecvThread():
    global AirBand
    global ExitRecvThread
    global RemoteVideoMode
    global SessionID
    global RestartDisplay

    UDP_IP = ""
    UDP_PORT = 8943 #2022 - UDP DownLink from Air
    CommandSock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    CommandSock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    CommandSock.settimeout(1)
    CommandSock.bind((UDP_IP, UDP_PORT))
    OldAirBand = ""

    while True:
        try:
            data, addr = CommandSock.recvfrom(1024) # buffer size is 1024 bytes
            #print( "Data: " + str(data) )
            if data == InMsgBand5:
                lock.acquire()
                AirBand = "5"
                lock.release()
                if OldAirBand != AirBand:
                    OldAirBand = AirBand
                    lock.acquire()
                    RestartDisplay = 1
                    lock.release()

            if data == InMsgBand10:
                lock.acquire()
                AirBand = "a"
                lock.release()
                if OldAirBand != AirBand:
                    OldAirBand = AirBand
                    lock.acquire()
                    RestartDisplay = 1
                    lock.release()

            if data == InMsgBand20:
                lock.acquire()
                AirBand = "0"
                lock.release()
                if OldAirBand != AirBand:
                    OldAirBand = AirBand
                    lock.acquire()
                    RestartDisplay = 1
                    lock.release()

            if data == InMsgCameraTypeRPi:
                RemoteVideoMode=1

            if data == InMsgCameraTypeRPiAndSecondary:
                RemoteVideoMode=2

            if data == InMsgCameraTypeSecondary:
                RemoteVideoMode=3


            try:
                InDataStr = data.decode("utf-8")
                if InDataStr.startswith("SessionID") == True:
                    result = InDataStr[9:41]
                    if SessionID != result:
                        #print("new session detected")
                        if SessionID == "0":
                            SessionID = result
                        else:
                            SessionID = result
                            #print("Restart display required")
                            lock.acquire()
                            RestartDisplay = 1
                            lock.release()
            except Exception as e:
                print("Data decode error: E Message:  "  + str(e))


        except socket.timeout:
            if ExitRecvThread == 1:
                print("Recv thread exiting...")
                break

    CommandSock.close()
    ExitRecvThread = 2
            

def CheckIfCardExists(PathToCard):
    fPath = Path(PathToCard)
    if fPath.exists() == False:
        return False
    return True


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

def ReadLocalBand(PathToFile):
    try:
        hFile = open(PathToFile, 'r')
        res=hFile.readline()
        print(res)
        hFile.close()
        if  res.startswith("0x00000000") == True:
            return 20
        if  res.startswith("0x0000000a") == True:
            return 10
        if  res.startswith("0x00000005") == True:
            return 5

    except Exception as e:
        print("Error: thrown exception while processing file \n" + PathToFile + " E Message:  "  + str(e))
        return -1
    return -1

def SwitchRemoteLocalBandTo(band):
    global AirBand
    SlaveCardOldBandList = []
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

    #Check if all Slave cards still exists.
    SlaveCardCount = len(SlaveCardList)
    SlaveCardActual = 0;
    for i in range(SlaveCardCount):
        if CheckIfCardExists(SlaveCardList[i]) == True:
            SlaveCardActual += 1

    if SlaveCardCount != SlaveCardActual:
        print("Slave cards count changed since script started. Exit script to restart.")
        return False

    for z in range(SlaveCardCount):
        SingleOldBand = ReadLocalBand(SlaveCardList[z])
        if SingleOldBand != -1:
            SlaveCardOldBandList.append(SingleOldBand)
        else:
            return False

    #switch secondary WiFI card to requested band
    for z in range(SlaveCardCount):
        if SwitchLocalBandTo(SlaveCardList[z], band) == False:
            print("Failed to switch secondary card to requested band. Abort.")
            return False

    #send "band switch" request to Air
    switched = 0
    ResendCount = 0
    StopResend = 0
    while StopResend != 1: #Resend until confirmation be received
        print("Received remote band:  " + AirBand)
        SendDataToAir(SendMsgBuf)
        sleep(0.4) #wait in msg
        lock.acquire()
        if band == 5 and AirBand == "5":
            switched = 1
            StopResend = 1
        if band == 10 and AirBand == "a":
            switched = 1
            StopResend = 1
        if band == 20 and AirBand == "0":
            switched = 1
            StopResend = 1
        lock.release()

        ResendCount += 1
        if ResendCount >= 9:
            StopResend = 1

    if switched == 1:
        #Switch confirmed. Switch primary card to requested band.
        switched = 0
        while switched == 0:  #In case of error try to switch again
            if SwitchLocalBandTo(PrimaryCardPath, band) == True:
                #print("Switch local true")
                switched = 1
            else:
                sleep(0.5)
    else:
        #switch not confirmed. Roll back slave cards
        print("Switch to requested band not confirmed. Rollback...") 
        for z in range(SlaveCardCount):
            print("Trying to set old band: " + str(SlaveCardOldBandList[z]) + " to card: " + SlaveCardList[z])
            if SwitchLocalBandTo(SlaveCardList[z], SlaveCardOldBandList[z]) == False:
                print("SwitchLocalBandTo + rollback - failed")
            else:
                print("Rolled back.")
        return False

    print("Done. ")
    return True


def FindCardPhyInitPath():
    global PrimaryCardPath
    global SlaveCardList

    for root, dirs, files in os.walk("/sys/kernel/debug/ieee80211/"):
        for varname in dirs:
            if varname.__contains__(PrimaryCardMAC) == True:
                PrimaryCardPath = root
                PrimaryCardPath = PrimaryCardPath + "/ath9k_htc/chanbw"
                print("Primary card path: " + PrimaryCardPath);

    #Confirm that chanbw file exist
    fPrimary = Path(PrimaryCardPath)
    if fPrimary.exists() == False:
        print("chanbw file for card with MAC ", PrimaryCardMAC,  " Not found ")
        return False

    #Get Slave card list
    for root, dirs, files in os.walk("/sys/kernel/debug/ieee80211/"):
        for filename in files:
            if filename.__contains__("chanbw") == True:
                tmp = root
                tmp = root + "/chanbw"
                if tmp != PrimaryCardPath:
                    SlaveCardList.append(tmp)
                    
 
    #Confirm that chanbw file exist
    SlaveCardCount = len(SlaveCardList)
    if SlaveCardCount > 0:
        print("Number of slave cards: ", SlaveCardCount);
        for i in range(SlaveCardCount):
            fSlave = Path(SlaveCardList[i])
            if fSlave.exists() == False:
                print(SlaveCardList[i] + " Not found");
                return False
            else:
                print("Slave card #: ", i, " path: ", SlaveCardList[i])
    else:
        print("Slave card not found. Switch BW with one card unsafe, exit")
        return False
    
    return True

def ExitScript(ExitCode):
    global ExitRecvThread
    global ExitRCThread

    ExitRecvThread = 1
    ExitRCThread = 1

    while ExitRCThread == 1:
        sleep(0.2)

    while ExitRecvThread == 1:
        sleep(0.2)

    exit(ExitCode)


def CheckBandRCValues():
    global CurrentBand
    if RC_Value >= Band20After and CurrentBand != 20 and RC_Value != 0:
        print("Switching to 20MHz...")
        if SwitchRemoteLocalBandTo(20) != False:
            CurrentBand = 20

    if RC_Value < Band10ValueMax and RC_Value > Band10ValueMin and CurrentBand != 10 and RC_Value != 0:
        print("Switching to 10MHz...")
        if SwitchRemoteLocalBandTo(10) != False:
            CurrentBand = 10

    if RC_Value <= Band5Below and CurrentBand != 5 and RC_Value != 0:
        print("Switching to 5MHz...")
        if SwitchRemoteLocalBandTo(5) != False:
            CurrentBand = 5


def SwitchLocalDisplayMode():
    global LocalVideoMode
    global RestartDisplay
    if RemoteVideoMode == 1 and LocalVideoMode != 1 or RestartDisplay == 1:
        LocalVideoMode=1
        if RestartDisplay == 1:
            lock.acquire()
            RestartDisplay = 0
            lock.release()

        try:
            os.system('/usr/local/share/RemoteSettings/Ground/KillForwardRTPSecondaryCamera.sh  > /dev/null 2>&1')
        except Exception as e:
            print("Exception. KillForwardRTPSecondaryCamera.sh: "  + str(e))

    if RemoteVideoMode == 2 and LocalVideoMode != 2 or RestartDisplay == 1:
        LocalVideoMode=2
        if RestartDisplay == 1:
            lock.acquire()
            RestartDisplay = 0
            lock.release()

        try:
            os.system('/usr/local/share/RemoteSettings/Ground/KillForwardRTPSecondaryCamera.sh  > /dev/null 2>&1')
        except Exception as e:
            print("Exception. KillForwardRTPSecondaryCamera.sh: " +  str(e))

        try:
            os.system('/usr/local/share/RemoteSettings/Ground/RxForwardSecondaryRTP.sh  > /dev/null 2>&1 &')
        except Exception as e:
            print("RxForwardSecondaryRTP.sh forward exception: " +  str(e))

    if RemoteVideoMode == 3 and LocalVideoMode != 3 or RestartDisplay == 1:
        LocalVideoMode=3
        if RestartDisplay == 1:
            lock.acquire()
            RestartDisplay = 0
            lock.release()

        try:
            os.system('/usr/local/share/RemoteSettings/Ground/KillForwardRTPSecondaryCamera.sh  > /dev/null 2>&1')
        except Exception as e:
            print("Exception. KillForwardRTPSecondaryCamera.sh: " +  str(e))

        try:
            os.system('/usr/local/share/RemoteSettings/Ground/RxForwardSecondaryRTPAndDisplayLocally.sh  > /dev/null 2>&1 &')
        except Exception as e:
            print("RxForwardSecondaryRTPAndDisplayLocally forward exception: " +  str(e))


def CheckSecondaryCameraRCValues():
    if RC_Value2 >= Camera1ValueMin and RC_Value2 <= Camera1ValueMax:
        if RemoteVideoMode != 1:
            for i in range(5):
                SendDataToAir("RPi")



    if RC_Value2 >= Camera2ValueMin and RC_Value2 <= Camera2ValueMax:
        if RemoteVideoMode != 2:
            for i in range(5):
                SendDataToAir("RPiAndSecondary")


    if RC_Value2 >= Camera3ValueMin and RC_Value2 <= Camera3ValueMax:
        if RemoteVideoMode != 3:
            for i in range(5):
                SendDataToAir("Secondary")

     
    if RC_Value2 >= Camera4ValueMin and RC_Value2 <= Camera4ValueMax:
        print("Camera: non")

if FindCardPhyInitPath() == True:
    print("Ok")
    RecvThread = threading.Thread(target=StartRecvThread)
    RecvThread.start()

    RC_UDP_IN_thread = threading.Thread(target=StartRCThreadIn)
    RC_UDP_IN_thread.start()

    RC_UDP_IN_thread2 = threading.Thread(target=StartRCThreadIn2)
    RC_UDP_IN_thread2.start()

    CurrentBandTmp = 0
    CurrentBand = 0
    try:
        hFile = open(PrimaryCardPath, "r")
        CurrentBandTmp = hFile.readline()
        print("CurrnetBand from chanbw: ", CurrentBandTmp)
        hFile.close()
    except:
        print("Primary card chanbw file missing.")
        ExitScript(2)

    if CurrentBandTmp == "0x00000000":
        CurrentBand = 20
    elif CurrentBandTmp == "0x0000000a":
        CurrentBand = 10
    elif CurrentBandTmp == "0x00000005":
        CurrentBand = 5
    else:
        CurrentBand = 20
    print("CurrentBand: ",CurrentBand)


    #Add command line in code
    while True:
        CheckBandRCValues()
        CheckSecondaryCameraRCValues()
        SwitchLocalDisplayMode()

        sleep(0.6)

else:
    print("Ath9k card not found. Band switch disabled. USB\IP camera switch still enabled.")

    RecvThread = threading.Thread(target=StartRecvThread)
    RecvThread.start()

    RC_UDP_IN_thread2 = threading.Thread(target=StartRCThreadIn2)
    RC_UDP_IN_thread2.start()

    while True:
        CheckSecondaryCameraRCValues()
        SwitchLocalDisplayMode()
        sleep(0.6)
