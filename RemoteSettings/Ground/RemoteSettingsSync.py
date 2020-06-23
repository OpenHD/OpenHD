import fileinput
import socket
import hashlib
import os
from time import sleep
import subprocess
import re
from sys import stdout
from datetime import datetime
import argparse
import RPi.GPIO as GPIO
from struct import *
import threading

from enum import Enum

class SmartSyncState(Enum):
    Initializing = 0
    WaitingForTrigger = 1
    WaitingForAir = 2
    Transferring = 3
    NotNeeded = 4
    Finished = 5
    Error = 6
    Skipped = 7

lock = threading.Lock()

parser = argparse.ArgumentParser()
parser.add_argument("-ControlVia", help="")
args = parser.parse_args()
ArgParam = args.ControlVia

SelectedControl = "0"

if ArgParam == "GPIO":
    SelectedControl="GPIO"
elif ArgParam == "joystick":
    SelectedControl="joystick"
else:
    SelectedControl = "joystick"
	
	
print("Selected: " + SelectedControl)

GPIO.setmode(GPIO.BCM)
GPIO.setup(26, GPIO.IN, pull_up_down=GPIO.PUD_UP)

GPIO.setup(20, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(21, GPIO.IN, pull_up_down=GPIO.PUD_UP)

input_state0 = GPIO.input(20)
input_state1 = GPIO.input(21)

SettingsFilePath = "/boot/openhd-settings-1.txt"

if (input_state0 == False) and (input_state1 == False):
    SettingsFilePath = "/boot/openhd-settings-4.txt"

if (input_state0 == False) and (input_state1 == True):
    SettingsFilePath = "/boot/openhd-settings-3.txt"

if (input_state0 == True) and (input_state1 == False):
    SettingsFilePath = "/boot/openhd-settings-2.txt"

if (input_state0 == True) and (input_state1 == True):
    SettingsFilePath = "/boot/openhd-settings-1.txt"

print(SettingsFilePath)


UDP_PORT_OUT = 1376
UDP_PORT_IN = 1375
#
# Goes to the status microservice
#
UDP_INFO_PORT_OUT=50000
#
# Used for OpenHDBoot at the moment
#
UDP_SMARTSYNC_STATE_PORT_OUT=50001


RecvSocket = 0
JoystickSettingsFilePath = "/boot/joyconfig.txt"
IsMainScriptRunning = False

WFBDefultFreq24="2412"
WFBDefultFreq58="5180"
FreqFromConfigFile="auto"

SmartSyncFreqDefault24 = "2412"
SmartSyncFreqDefault58 = "5180"
SmartSyncFreqFromConfigFile="auto"

WlanName = "0"
RC_Value = 0
ExitRCThread = 0
StartTime = 0
LastRequestTime = 0

SmartSyncOFF_RC_Value=1700
SmartSyncStayON_RC_Value=1300
SmartSyncRC_Channel=3
SmartSync_StartupMode=1
SmartSyncGround_Countdown=45
NotBreakByTimerIfLastRequestWas=3


TxPowerConfigFilePath="/etc/modprobe.d/ath9k_hw.conf"
TxPowerFromConfig="-1"
TxPowerFromAth9k_hw="-1"

RequestGroundChecksum = bytearray(b'RequestGroundChecksum')
RequestSFile = bytearray(b'RequestSFile')
AirToGroundACK = bytearray(b'AirToGroundACK')
DownloadFinished = bytearray(b'DownloadFinished')
NoNeedInSync = bytearray(b'NoNeedInSync')


sockToAir = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sockToStatusMicroservice = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sockToOpenHDBoot = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def md5(fname):
    try:
        hash_md5 = hashlib.md5()
        with open(fname, "rb") as f:
            for chunk in iter(lambda: f.read(4096), b""):
                hash_md5.update(chunk)
        return hash_md5.hexdigest()
    except Exception as e:
       return False


def SendData(MessageBuf):
    try:
        
        #sockToAir.sendto( bytes(MessageBuf,'utf-8'), ('127.0.0.1', UDP_PORT_OUT))
        sockToAir.sendto(MessageBuf, ('127.0.0.1', UDP_PORT_OUT))
    except Exception as e:
        return False


def SendInfoToDisplay(Level, MessageBuf):
    try:
        print(MessageBuf)
        msg = pack('B 50s', Level, MessageBuf.encode('utf-8'))
        sockToStatusMicroservice.sendto(msg, ('127.0.0.1', UDP_INFO_PORT_OUT))
    except Exception as e:
        print(e)
        return False


def SendSmartSyncState(State, Progress):
    try:
        GPIOState = GPIO.input(26)
        msg = pack('i i i i i i i', State.value, Progress, SmartSyncRC_Channel, SmartSyncStayON_RC_Value, SmartSyncOFF_RC_Value, RC_Value, GPIOState)
        sockToOpenHDBoot.sendto(msg, ('127.0.0.1', UDP_SMARTSYNC_STATE_PORT_OUT))
    except Exception as e:
        print(e)
        return False


def ReadFileFrom(offset, BytesToRead):
    try:
        f = open(SettingsFilePath, "rb")
        f.seek(offset)
        text = f.read(BytesToRead)
        f.close()
        return text
    except Exception as e:
        return False

def GetPrimaryCardMAC_Config():
    try:
        with open(SettingsFilePath, "r") as f:
            lines = f.readlines()
            for line in lines:
                if line.startswith("PrimaryCardMAC=") == True:
                    SplitLines = line.split("=")
                    linezero = SplitLines[1]
                    result = re.sub(r'[^a-zA-Z0-9 ]',r'', linezero)
                    return result

    except Exception as e:
       return False
    return False



def ReadJoystickConfigFile():
    ROLL_AXIS = "0"
    PITCH_AXIS = "1"
    YAW_AXIS = "3"
    THROTTLE_AXIS = "2"
    AUX1_AXIS = "4"
    AUX2_AXIS = "5"
    AUX3_AXIS = "6"
    AUX4_AXIS  = "7"
    
    try:
        with open(JoystickSettingsFilePath, "r") as f:
            lines = f.readlines()
            for line in lines:
                if line.startswith("#define ROLL_AXIS") == True:
                    SplitLines = line.split()
                    FilterDigits = SplitLines[2]
                    ROLL_AXIS = re.sub("\D", "", FilterDigits)

                if line.startswith("#define PITCH_AXIS") == True:
                    SplitLines = line.split()
                    FilterDigits = SplitLines[2]
                    PITCH_AXIS = re.sub("\D", "", FilterDigits)

                if line.startswith("#define YAW_AXIS") == True:
                    SplitLines = line.split()
                    FilterDigits = SplitLines[2]
                    YAW_AXIS = re.sub("\D", "", FilterDigits)

                if line.startswith("#define THROTTLE_AXIS") == True:
                    SplitLines = line.split()
                    FilterDigits = SplitLines[2]
                    THROTTLE_AXIS = re.sub("\D", "", FilterDigits)

                if line.startswith("#define AUX1_AXIS") == True:
                    SplitLines = line.split()
                    FilterDigits = SplitLines[2]
                    AUX1_AXIS = re.sub("\D", "", FilterDigits)

                if line.startswith("#define AUX2_AXIS") == True:
                    SplitLines = line.split()
                    FilterDigits = SplitLines[2]
                    AUX2_AXIS = re.sub("\D", "", FilterDigits)

                if line.startswith("#define AUX3_AXIS") == True:
                    SplitLines = line.split()
                    FilterDigits = SplitLines[2]
                    AUX3_AXIS = re.sub("\D", "", FilterDigits)

                if line.startswith("#define AUX4_AXIS") == True:
                    SplitLines = line.split()
                    FilterDigits = SplitLines[2]
                    AUX4_AXIS = re.sub("\D", "", FilterDigits)


        return  {'ROLL_AXIS':ROLL_AXIS, 'PITCH_AXIS':PITCH_AXIS ,'YAW_AXIS':YAW_AXIS,    'THROTTLE_AXIS':THROTTLE_AXIS, 'AUX1_AXIS':AUX1_AXIS ,'AUX2_AXIS':AUX2_AXIS,    'AUX3_AXIS':AUX3_AXIS, 'AUX4_AXIS':AUX4_AXIS }

    except Exception as e:
       print(e)

    return False


def ReadSettingsFromConfigFile():
    global SmartSyncOFF_RC_Value
    global SmartSyncStayON_RC_Value
    global SmartSyncRC_Channel
    global SmartSync_StartupMode
    global SmartSyncGround_Countdown
    global FreqFromConfigFile
    global SmartSyncFreqFromConfigFile

    try:
        with open(SettingsFilePath, "r") as f:
            lines = f.readlines()
            for line in lines:
                if line.startswith("FREQ=") == True:
                    SplitLines = line.split("=")
                    FreqFromConfigFile = SplitLines[1]

                if line.startswith("SmartSyncOFF_RC_Value") == True:
                    SplitLines = line.split("=")
                    FilterDigits = SplitLines[1]
                    SmartSyncOFF_RC_Value = int(re.sub("\D", "", FilterDigits) )

                if line.startswith("SmartSyncStayON_RC_Value") == True:
                    SplitLines = line.split("=")
                    FilterDigits = SplitLines[1]
                    SmartSyncStayON_RC_Value = int(re.sub("\D", "", FilterDigits) )

                if line.startswith("SmartSyncRC_Channel") == True:
                    SplitLines = line.split("=")
                    FilterDigits = SplitLines[1]
                    SmartSyncRC_Channel = int(re.sub("\D", "", FilterDigits) )

                if line.startswith("SmartSync_StartupMode") == True:
                    SplitLines = line.split("=")
                    FilterDigits = SplitLines[1]
                    SmartSync_StartupMode = int(re.sub("\D", "", FilterDigits) )

                if line.startswith("SmartSyncGround_Countdown") == True:
                    SplitLines = line.split("=")
                    FilterDigits = SplitLines[1]
                    SmartSyncGround_Countdown = int(re.sub("\D", "", FilterDigits) )
                    
                if line.startswith("SmartSync_Frequency=") == True:
                    SplitLines = line.split("=")
                    FilterDigits = SplitLines[1]
                    SmartSyncFreqFromConfigFile = re.sub("\D", "", FilterDigits) 

            return True


    except Exception as e:
       SendInfoToDisplay(3, "SmartSync: error occurred while reading settings file")
       return False
    return False


def FindWlanNameByMAC(PrimaryCardMAC):
    try:
        for root, dirs, files in os.walk("/sys/class/net/"):
            for dir in dirs:
                filePath = "/sys/class/net/" + dir + "/address"
                with open(filePath, "r") as f:
                    lines = f.readlines()
                    digits = re.sub(r'[^a-zA-Z0-9 ]',r'',lines[0])
                    if digits == PrimaryCardMAC:
                        return dir

    except Exception as e:
       return False
    return False



def FindWlanToUseGround():
    global WlanName

    SmartSyncFreq = SelectSmartSyncFrequency()

    PrimaryCardMAC = GetPrimaryCardMAC_Config()
    if PrimaryCardMAC == False or PrimaryCardMAC == "0":
        SendInfoToDisplay(5, "SmartSync: trying to initialize WLAN...")
        try:
            for root, dirs, files in os.walk("/sys/class/net/"):
                for dir in dirs:
                    if dir.startswith("eth") == False and  dir.startswith("lo") == False and  dir.startswith("usb") == False and  dir.startswith("intwifi") == False and  dir.startswith("relay") == False and dir.startswith("wifihotspot") == False:
                        SendInfoToDisplay(5, "Found WLan with name: " + dir)
                        WlanName = dir
            if WlanName != "0":
                SendInfoToDisplay(5, "SmartSync: using WLAN interface " + WlanName)
                subprocess.check_call(['/sbin/iw', "dev", WlanName , "set", "freq", SmartSyncFreq ])
                SendInfoToDisplay(5, "SmartSync: frequency set to " + SmartSyncFreq)
                return True
            else:
                return False

        except Exception as e:
            return False
    else:
        SendInfoToDisplay(5, "SmartSync: trying to find wlan with MAC:" + PrimaryCardMAC)
        result = FindWlanNameByMAC(PrimaryCardMAC)
        if result != False:
            SendInfoToDisplay(5, "SmartSync: wlan with primary card MAC found ")
            try:
                WlanName = result
                SendInfoToDisplay(5, "SmartSync: using WLAN interface " + WlanName)
                subprocess.check_call(['/sbin/iw', "dev", WlanName , "set", "freq", SmartSyncFreq])
                SendInfoToDisplay(5, "SmartSync: frequency set to " + SmartSyncFreq)
                return True
            except Exception as e:
                return False
        else:
            SendInfoToDisplay(5, "SmartSync: wlan with MAC" + PrimaryCardMAC +  "not found, looking for another...")
            try:
                for root, dirs, files in os.walk("/sys/class/net/"):
                    for dir in dirs:
                        if dir.startswith("eth") == False and  dir.startswith("lo") == False and  dir.startswith("usb") == False and  dir.startswith("intwifi") == False and  dir.startswith("relay") == False and dir.startswith("wifihotspot") == False:
                            SendInfoToDisplay(5, "SmartSync: found WLan with name: " + dir)
                            WlanName = dir
                if WlanName != "0":
                    SendInfoToDisplay(5, "SmartSync: using WLAN interface "  + WlanName)
                    subprocess.check_call(['/sbin/iw', "dev", WlanName , "set", "freq", SmartSyncFreq])
                    SendInfoToDisplay(5, "SmartSync: frequency set to " + SmartSyncFreq)
                    return True
                else:
                    return False

            except Exception as e:
                return False

#Not used. Can be used with ath9k driver reload.
def InitWlan():
    global WlanName
    SmartSyncFreq = SelectSmartSyncFrequency()
    PrimaryCardMAC = GetPrimaryCardMAC_Config()
    if PrimaryCardMAC == False or PrimaryCardMAC == "0":
        SendInfoToDisplay(5, "SmartSync: trying to initialize wlan0...")
        try:
            if os.path.isdir("/sys/class/net/wlan0") == True:
                WlanName = "wlan0"
                subprocess.check_call(['/usr/local/share/RemoteSettings/Ground/SetWlanXMonitorModeFreq.sh', "wlan0", SmartSyncFreq ])
                return True
            else:
                SendInfoToDisplay(3, "SmartSync: wlan0 not found")
                return False
        except Exception as e:
            return False
    else:
        SendInfoToDisplay(5, "SmartSync: configuring wlan with MAC: " + PrimaryCardMAC)
        result = FindWlanNameByMAC(PrimaryCardMAC)
        if result != False:
            SendInfoToDisplay(5, "SmartSync: wlan name with MAC: " + result)
            try:
                WlanName = result
                subprocess.check_call(['/usr/local/share/RemoteSettings/Ground/SetWlanXMonitorModeFreq.sh', result, SmartSyncFreq ])
                return True
            except Exception as e:
                return False
        else:
            SendInfoToDisplay(5, "SmartSync: wlan with primary card MAC not found, trying wlan0...")
            try:
                if os.path.isdir("/sys/class/net/wlan0") == True:
                    WlanName = "wlan0"
                    subprocess.check_call(['/usr/local/share/RemoteSettings/Ground/SetWlanXMonitorModeFreq.sh', "wlan0", SmartSyncFreq ])
                    return True
                else:
                    SendInfoToDisplay(3, "SmartSync: wlan0 not found")
                    return False
            except Exception as e:
                return False

def ShutDownWlan():
    try:
        subprocess.check_call(['/bin/ip', "link" ,"set", WlanName, "down" ])
        return True
    except Exception as e:
        return False
    return False

def IsWlanAth9k():
    try:
        fPath = "/sys/class/net/" + WlanName + "/device/uevent"
        with open(fPath, "r") as f:
            lines = f.readlines()
            for line in lines:
                if "ath9k_htc" in line:
                    SendInfoToDisplay(5, "SmartSync: wlan is ath9k_htc, driver reload required")
                    return True
        return False
    except Exception as e:
        return False
    return False


def UnloadAth9kDriver():
    SendInfoToDisplay(5, "SmartSync: unload Ath9k_htc Driver...")
    try:
        subprocess.check_call(['rmmod', "ath9k_htc"  ])
        subprocess.check_call(['rmmod', "ath9k_common"  ])
        subprocess.check_call(['rmmod', "ath9k_hw"  ])
        subprocess.check_call(['rmmod', "ath"  ])
        return True
    except Exception as e:
        return False
    return False

def LoadAth9kDriver():
    SendInfoToDisplay(5, "SmartSync: load Ath9k_htc Driver...")
    try:
        subprocess.check_call(['modprobe', "ath9k_htc"  ])
        subprocess.check_call(['modprobe', "ath9k_common"  ])
        subprocess.check_call(['modprobe', "ath9k_hw"  ])
        subprocess.check_call(['modprobe', "ath"  ])
        return True
    except Exception as e:
        return False
    return False

def CreateFinishMarkFile():
    try:
        f = open("/tmp/ReadyToGo", "w")
        f.write("1")
        f.close()
        return True
    except Exception as e:
        return False
    return False

def InitDevNull():
    global RxDevNull
    global RCDevNull
    try:
        RxDevNull = open(os.devnull, 'w')
        RCDevNull = open(os.devnull, 'w')
        return True
    except Exception as e:
        return False
    return False

def DetectWFBPrimaryBand():
    ret = os.system('lsmod | grep 88XXau')
    if ret == 0:
        return "58"
    else:
        return "24"

def GetDefaultWFBFrequency():
    WFBPrimaryBand = DetectWFBPrimaryBand()
    if WFBPrimaryBand == "58":
        return WFBDefultFreq58
    elif WFBPrimaryBand == "24":
        return WFBDefultFreq24
    else:
        # we have no way of knowing what to use because the supported bands aren't known
        return WFBDefultFreq24

def SelectSmartSyncFrequency():
    WFBPrimaryBand = DetectWFBPrimaryBand()
    try:
        if not SmartSyncFreqFromConfigFile or "auto" in SmartSyncFreqFromConfigFile:
            # either the file couldn't be read or the user still has auto chosen, so we pick for them
            WFBPrimaryBand = DetectWFBPrimaryBand()
            if WFBPrimaryBand == "58":
                return SmartSyncFreqDefault58
            elif WFBPrimaryBand == "24":
                return SmartSyncFreqDefault24
            else:
                # we have no way of knowing what to use because the supported bands aren't known
                return SmartSyncFreqDefault24
        else:
            # the user changed the smartsync frequency, we use it but they're on their own now
            return SmartSyncFreqFromConfigFile
    except Exception as e:
       print(e)
       return False
    return False


def StartSVPcomTx():                                     
    try:                                     
        subprocess.Popen(['/usr/local/share/cameracontrol/IPCamera/svpcom_wifibroadcast/wfb_tx',"-k", "1", "-n", "1",
                          "-K", "/usr/local/share/cameracontrol/IPCamera/svpcom_wifibroadcast/tx.key",
                          "-u" ,str(UDP_PORT_OUT), "-p", "93", "-B", "20", "-M", "0", WlanName ])
        return True
    except Exception as e:
        return False
    return False

def StartSVPcomRx():   
    try:                                     
        subprocess.Popen( ['/usr/local/share/cameracontrol/IPCamera/svpcom_wifibroadcast/wfb_rx', "-k", "1", "-n", "1",
                               "-K", "/usr/local/share/cameracontrol/IPCamera/svpcom_wifibroadcast/rx.key",
                              "-c" ,"127.0.0.1", "-u", str(UDP_PORT_IN), "-p", "92",  WlanName ], stdout=RxDevNull)
        return True
    except Exception as e:
        return False
    return False

def StartRC_Reader(ChannelToRead):
    ROLL_AXIS = "0"
    PITCH_AXIS = "1"
    YAW_AXIS = "3"
    THROTTLE_AXIS = "2"
    AUX1_AXIS = "4"
    AUX2_AXIS = "5"
    AUX3_AXIS = "6"
    AUX4_AXIS  = "7"

    result = ReadJoystickConfigFile()
    if result == False:
        SendInfoToDisplay(4, "SmartSync: can`t read joystick config file, using defaults")
    else:

        ROLL_AXIS = result['ROLL_AXIS']
        PITCH_AXIS = result['PITCH_AXIS']
        YAW_AXIS = result['YAW_AXIS']
        THROTTLE_AXIS = result['THROTTLE_AXIS']
        AUX1_AXIS = result['AUX1_AXIS']
        AUX2_AXIS = result['AUX2_AXIS']
        AUX3_AXIS = result['AUX3_AXIS']
        AUX4_AXIS  = result['AUX4_AXIS']

    try:
        if ChannelToRead > 16:
            SendInfoToDisplay(4, "SmartSync: selected RC channel greater than 16.  Forced to channel 1")
            ChannelToRead = 1
        if ChannelToRead < 1:
           SendInfoToDisplay(4, "SmartSync: selected RC channel less than 1.  Forced to channel 1")
           ChannelToRead = 1

        subprocess.Popen( ['/usr/local/share/RemoteSettings/Ground/helper/JoystickSender', str(ChannelToRead), ROLL_AXIS, PITCH_AXIS,YAW_AXIS, THROTTLE_AXIS,AUX1_AXIS,AUX2_AXIS,AUX3_AXIS,AUX4_AXIS ], stdout=RCDevNull)
        return True
    except Exception as e:
        return False
    return False

def IsTimeToExitByTimer():
    if StartTime != 0:
        TimeNow = datetime.now()
        diff = (TimeNow - StartTime).total_seconds()
        if diff > 5:
            if diff > SmartSyncGround_Countdown:
                if LastRequestTime != 0:
                    diffLastRequest = (TimeNow - LastRequestTime).total_seconds()
                    if NotBreakByTimerIfLastRequestWas > diffLastRequest:
                        SendInfoToDisplay(5, "SmartSync: in progress.... timer delayed.")
                        return False
                    
                SendInfoToDisplay(4, "SmartSync: interrupted by RC timer")
                return True
    return False

def IsTimeToExitByTimeout():
    if LastRequestTime != 0:
        TimeNow = datetime.now()
        diffLastRequest = (TimeNow - LastRequestTime).total_seconds()
        if diffLastRequest > NotBreakByTimerIfLastRequestWas:
            SendInfoToDisplay(4, "SmartSync: sync timeout")
            return True
    return False


def IsTimeToExit():
    global ExitRCThread

    if RC_Value >= SmartSyncOFF_RC_Value and RC_Value != 0:
        SendInfoToDisplay(4, "SmartSync: interrupted by RC joystick")
        ExitRCThread = 1
        return True
    else:
        if SelectedControl == "GPIO":
            GPIOState = GPIO.input(26)
            if(GPIOState == True):
                SendInfoToDisplay(4, "SmartSync: interrupted by GPIO")
                ExitRCThread = 1
                return True

    return False

def InitUDPServer():
    for i in range(0,10):
        sleep(0.02)

    global RecvSocket
    global IsMainScriptRunning
    global LastRequestTime
    UDP_IP = ""
    try:
        RecvSocket = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
        RecvSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        RecvSocket.settimeout(0.5)
        RecvSocket.bind((UDP_IP, UDP_PORT_IN))
    except Exception as e:
        return False

    MessageBufFile =  bytearray()
    while True:
        try:
            if IsTimeToExit() == True or IsTimeToExitByTimer() == True or IsTimeToExitByTimeout() == True:
                #if IsWlanAth9k() == True:
                #    UnloadAth9kDriver()
                #    sleep(2)
                #    LoadAth9kDriver()
                #    sleep(4)
                #else:
                #    ShutDownWlan()
                CleanAndExit()
                break
            MessageBufFile.clear()
            data, addr = RecvSocket.recvfrom(200)
            if data == RequestGroundChecksum:
                #SendMessage: 32 bytes md5 + 5 bytes file size
                LastRequestTime = datetime.now()
                result = md5(SettingsFilePath)
                if result != False:

                    md5Bytes = result.encode('ascii')
                    SizeInBytesInt = os.path.getsize(SettingsFilePath)
                    if SizeInBytesInt != 0:
                        MessageBufFile.extend(md5Bytes)
                        SizeInBytesString = str(SizeInBytesInt)
                        SizeInBytesByte = SizeInBytesString.encode('ascii')
                        MessageBufFile.extend(SizeInBytesByte)
                        SendData(MessageBufFile)
                        MessageBufFile.clear()
            if RequestSFile in data:
                LastRequestTime = datetime.now()
                #return: 6 bytes - offset that was read + data 1024 bytes or less
                if len(data) > 13:
                    offset = data[13:len(data)]
                    offsetInt = int(offset)

                    SizeInBytes = os.path.getsize(SettingsFilePath)
                    BytesTillEndOfFile = SizeInBytes - offsetInt

                    Transferred = SizeInBytes - BytesTillEndOfFile
                    PercentFinished = (float(Transferred) / float(SizeInBytes)) * 100.0
                    
                    SendInfoToDisplay(5, "SmartSync: transferred {:d}%".format(int(PercentFinished)))

                    SendSmartSyncState(SmartSyncState.Transferring, int(PercentFinished))

                    headerStr =  '{:0>6}'.format(offsetInt)
                    header = headerStr.encode('ascii')
                    if BytesTillEndOfFile >= 1024:
                        MessageBufFile.extend(header)
                        tmp = ReadFileFrom(offsetInt,1024)
                        MessageBufFile.extend(tmp)
                        SendData(MessageBufFile)
                        MessageBufFile.clear()
                    else:
                        MessageBufFile.extend(header)
                        MessageBufFile.extend(ReadFileFrom(offsetInt,BytesTillEndOfFile))
                        SendData(MessageBufFile)
                        MessageBufFile.clear()

            if data == DownloadFinished:
                LastRequestTime = datetime.now()
                tmp = "ACK".encode('ascii')
                for i in range(0,10):
                    SendData(tmp)
                    sleep(0.05)
                MessageBufFile.clear()
                RecvSocket.close()

                #if IsWlanAth9k() == True:
                #    UnloadAth9kDriver()
                #    sleep(2)
                #    LoadAth9kDriver()
                #    sleep(4)
                #else:
                #    ShutDownWlan()

                CreateFinishMarkFile()
                SendInfoToDisplay(5, "SmartSync: finished")
                SendSmartSyncState(SmartSyncState.Finished, 100)
                CleanAndExit()
                exit()
                

            if data == NoNeedInSync:
                LastRequestTime = datetime.now()
                SendInfoToDisplay(5, "SmartSync: no sync needed")
                SendSmartSyncState(SmartSyncState.NotNeeded, 100)
                tmp = "ACK".encode('ascii')
                for i in range(0,10):
                    SendData(tmp)
                    sleep(0.05)
                MessageBufFile.clear()
                RecvSocket.close()

                #if IsWlanAth9k() == True:
                #    UnloadAth9kDriver()
                #    sleep(2)
                #    LoadAth9kDriver()
                #    sleep(4)
                #else:
                #   ShutDownWlan()

                CreateFinishMarkFile()
                CleanAndExit()
                exit()


        except Exception as e:
            pass

    return False


def StartRCThreadIn():
    global RC_Value
    global ExitRCThread
    UDP_IP = ""
    UDP_PORT = 1260
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
            if ExitRCThread == 1:
                break
        except:
            if ExitRCThread == 1:
                break

    RCSock.close()


def ReturnWlanFreq():
    global FreqFromConfigFile
    if WlanName != "0":
        try:
            if not FreqFromConfigFile or "auto" in FreqFromConfigFile:
                WFBFreq = GetDefaultWFBFrequency()
                subprocess.check_call(['/usr/local/share/RemoteSettings/Ground/SetWlanFreq.sh', WlanName , WFBFreq ])
                SendInfoToDisplay(5, "Using automatic WFB frequency: " + WFBFreq)
            else:
                FreqFromConfigFile = re.sub("\D", "", FreqFromConfigFile) 
                subprocess.check_call(['/usr/local/share/RemoteSettings/Ground/SetWlanFreq.sh', WlanName , FreqFromConfigFile ])
                SendInfoToDisplay(5, "SmartSync: setting  " + WlanName + " to: " + FreqFromConfigFile)
        except Exception as e:
            SendInfoToDisplay(3, "SmartSync: error setting frequency to " + FreqFromConfigFile)

def CleanAndExit():
    global ExitRCThread
    global RxDevNull
    global RCDevNull

    ExitRCThread = 1
    ReturnWlanFreq()
    sleep(1)

    try:
        subprocess.check_call(['/usr/bin/killall', "JoystickSender" ]) 
    except Exception as e:
        pass

    try:
        subprocess.check_call(['/usr/bin/killall', "wfb_rx" ]) 
    except Exception as e:
        pass

    try:
        subprocess.check_call(['/usr/bin/killall', "wfb_tx" ]) 
    except Exception as e:
        pass


    RxDevNull.close()
    RCDevNull.close()
    exit()

def ShowSettings():
    SendInfoToDisplay(5, "SmartSync: RC value to disable is " + str(SmartSyncOFF_RC_Value) )
    SendInfoToDisplay(5, "SmartSync: RC value to enable is " + str(SmartSyncStayON_RC_Value) )
    SendInfoToDisplay(5, "SmartSync: RC channel is " + str(SmartSyncRC_Channel) )

    if SmartSync_StartupMode == 1:
        SendInfoToDisplay(5, "SmartSync: using force wait for air mode")
    else:
        SendInfoToDisplay(5, "SmartSync: using RC/GPIO trigger mode")


def ReadTxPowerAth9k_hw():
    global TxPowerFromAth9k_hw
    try:
        with open(TxPowerConfigFilePath, "r") as f:
            lines = f.readlines()
            for line in lines:
                if line.startswith("options ath9k_hw txpower") == True:
                    SplitLines = line.split("=")
                    FilterDigits = SplitLines[1]
                    TxPowerFromAth9k_hw = re.sub("\D", "", FilterDigits)

            return True

    except Exception as e:
       print(e)
       return False
    return False

def ReadTxPower():
    global TxPowerFromConfig
    try:
        with open(SettingsFilePath, "r") as f:
            lines = f.readlines()
            for line in lines:
                if line.startswith("TxPowerGround") == True:
                    SplitLines = line.split("=")
                    FilterDigits = SplitLines[1]
                    TxPowerFromConfig = re.sub("\D", "", FilterDigits)

            return True

    except Exception as e:
       print(e)
       return False
    return False

def CheckTxPower():
    try:
        if ReadTxPowerAth9k_hw() != False:
            print("TxPowerFromAth9k_hw= " + TxPowerFromAth9k_hw)
            if ReadTxPower() != False:
                print("TxPowerFromConfig= " + TxPowerFromConfig)
                if TxPowerFromConfig != TxPowerFromAth9k_hw:
                    print("TxPower not equal Check if all ok and apply")
                    if TxPowerFromAth9k_hw != "-1" and TxPowerFromConfig != "-1":
                        print("all ok, apply")
                        subprocess.check_call(['/usr/local/bin/txpower_atheros', TxPowerFromConfig ] )
                        return True
    except Exception as e:
        print(e)
        return False
    return False

#################################################### start

CheckTxPower()


if os.path.isfile("/tmp/ReadyToGo") == True:
    exit()

InitDevNull()

RC_UDP_IN_thread = threading.Thread(target=StartRCThreadIn)
RC_UDP_IN_thread.start()

if ReadSettingsFromConfigFile() == True:
    ShowSettings()
else:
    SendInfoToDisplay(3, "SmartSync: error reading settings, check config file")
    SmartSyncOFF_RC_Value=1700
    SmartSyncStayON_RC_Value=1400
    SmartSyncRC_Channel=3
    SmartSync_StartupMode=1
    SmartSyncGround_Countdown=45
    NotBreakByTimerIfLastRequestWas=3
    ShowSettings()

StartRC_Reader(SmartSyncRC_Channel)

SendInfoToDisplay(5, "SmartSync: initializing")

if SmartSync_StartupMode != 1:
    SendSmartSyncState(SmartSyncState.WaitingForTrigger, 0)

    for i in range(0, 10):
        #SendInfoToDisplay(6, "SmartSync: I is:", i, "RC value: ", RC_Value)
        #stdout.write("\r RC value: "+  str(RC_Value) + " Retry: " + str(i) + " of 30")
        #stdout.flush()
        SendInfoToDisplay(5, "SmartSync: RC value: "+  str(RC_Value) + " Retry: " + str(i) + " of 10")
        if RC_Value <= SmartSyncStayON_RC_Value and RC_Value != 0:
            SmartSync_StartupMode = 1
            SmartSyncGround_Countdown=0
            SendInfoToDisplay(5, "SmartSync: forced to sync due to joystick")
            SendSmartSyncState(SmartSyncState.WaitingForAir, 0)
            break
        if RC_Value >= SmartSyncOFF_RC_Value and RC_Value != 0:
            SmartSync_StartupMode = 0
            SendInfoToDisplay(5, "SmartSync: forced to skip due to joystick")
            SendSmartSyncState(SmartSyncState.Skipped, 100)
            break
        GPIOState = GPIO.input(26)
        if(GPIOState == False):
            SmartSync_StartupMode = 1
            SmartSyncGround_Countdown=0
            SendInfoToDisplay(5, "SmartSync: forced to sync due to GPIO")
            SendSmartSyncState(SmartSyncState.WaitingForAir, 0)
            break

        sleep(0.3)


if SmartSync_StartupMode == 1:
    SendSmartSyncState(SmartSyncState.WaitingForAir, 0)

    #if InitWlan() != False:
    if FindWlanToUseGround() != False:
        if StartSVPcomRx() != False:
            if StartSVPcomTx() != False:
                if SmartSyncGround_Countdown > 5:
                    SendInfoToDisplay(5, "SmartSync: starting timer")
                    StartTime = datetime.now()
                InitUDPServer()
            else:
                SendInfoToDisplay(3, "SmartSync: can't initialize radio TX")
                SendSmartSyncState(SmartSyncState.Error, 0)
        else:
            SendInfoToDisplay(3, "SmartSync: can't initialize radio RX")
            SendSmartSyncState(SmartSyncState.Error, 0)
    
    else:
        SendInfoToDisplay(3, "SmartSync: can't initialize wlan interface")
        SendSmartSyncState(SmartSyncState.Error, 0)
else:
    SendInfoToDisplay(4, "SmartSync: disabled")
    SendSmartSyncState(SmartSyncState.Skipped, 100)

CleanAndExit()
