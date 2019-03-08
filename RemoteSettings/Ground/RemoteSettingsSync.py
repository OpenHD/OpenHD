import fileinput
import socket
import hashlib
import os
from time import sleep
import subprocess
import re
from sys import stdout
from datetime import datetime
import threading
lock = threading.Lock()



UDP_PORT_OUT = 1376
UDP_PORT_IN = 1375
UDP_INFO_PORT_OUT=1379
RecvSocket = 0
SettingsFilePath = "/boot/openhd-settings-1.txt"
IsMainScriptRunning = False
DefaultCommunicateFreq = "2412"
FreqFromConfigFile= "0"
WlanName = "0"
RC_Value = 0
ExitRCThread = 0
StartTime = 0
LastRequestTime = 0

SettingsSyncForceOffRC_Value=1700
SettingsSyncForceOnRC_Value=1400
SettingsSyncRC_Channel=1
SettingsSyncStartOption=1
SettingsSyncOffByTimer=20
NotBreakByTimerIfLastRequestWas=3


RequestGroundChecksum = bytearray(b'RequestGroundChecksum')
RequestSFile = bytearray(b'RequestSFile')
AirToGroundACK = bytearray(b'AirToGroundACK')
DownloadFinished = bytearray(b'DownloadFinished')
NoNeedInSync = bytearray(b'NoNeedInSync')

def md5(fname):
    try:
        hash_md5 = hashlib.md5()
        with open(fname, "rb") as f:
            for chunk in iter(lambda: f.read(4096), b""):
                hash_md5.update(chunk)
        return hash_md5.hexdigest()
    except Exception as e:
       SendInfoToDisplay(e)
       return False

sockToAir = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
def SendData(MessageBuf):
    try:
        
        #sockToAir.sendto( bytes(MessageBuf,'utf-8'), ('127.0.0.1', UDP_PORT_OUT))
        sockToAir.sendto(MessageBuf, ('127.0.0.1', UDP_PORT_OUT))
    except Exception as e:
        SendInfoToDisplay(e)
        return False

sockToAir = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
def SendInfoToDisplay(MessageBuf):
    try:
        print(MessageBuf)
        sockToAir.sendto( bytes(MessageBuf,'ascii'), ('127.0.0.1', UDP_INFO_PORT_OUT))
        #sockToAir.sendto(MessageBuf, ('127.0.0.1', UDP_INFO_PORT_OUT))
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
        SendInfoToDisplay(e)
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
       SendInfoToDisplay(e)
       return False
    return False


def ReadSettingsFromConfigFile():
    global SettingsSyncForceOffRC_Value
    global SettingsSyncForceOnRC_Value
    global SettingsSyncRC_Channel
    global SettingsSyncStartOption
    global SettingsSyncOffByTimer
    global FreqFromConfigFile
    try:
        with open(SettingsFilePath, "r") as f:
            lines = f.readlines()
            for line in lines:
                if line.startswith("FREQ=") == True:
                    SplitLines = line.split("=")
                    FilterDigits = SplitLines[1]
                    FreqFromConfigFile = re.sub("\D", "", FilterDigits) 

                if line.startswith("SettingsSyncForceOffRC_Value") == True:
                    SplitLines = line.split("=")
                    FilterDigits = SplitLines[1]
                    SettingsSyncForceOffRC_Value = int(re.sub("\D", "", FilterDigits) )

                if line.startswith("SettingsSyncForceOnRC_Value") == True:
                    SplitLines = line.split("=")
                    FilterDigits = SplitLines[1]
                    SettingsSyncForceOnRC_Value = int(re.sub("\D", "", FilterDigits) )

                if line.startswith("SettingsSyncRC_Channel") == True:
                    SplitLines = line.split("=")
                    FilterDigits = SplitLines[1]
                    SettingsSyncRC_Channel = int(re.sub("\D", "", FilterDigits) )

                if line.startswith("SettingsSyncStartOption") == True:
                    SplitLines = line.split("=")
                    FilterDigits = SplitLines[1]
                    SettingsSyncStartOption = int(re.sub("\D", "", FilterDigits) )

                if line.startswith("SettingsSyncOffByTimer") == True:
                    SplitLines = line.split("=")
                    FilterDigits = SplitLines[1]
                    SettingsSyncOffByTimer = int(re.sub("\D", "", FilterDigits) )

                if line.startswith("NotBreakByTimerIfLastRequestWas") == True:
                    SplitLines = line.split("=")
                    FilterDigits = SplitLines[1]
                    NotBreakByTimerIfLastRequestWas = int(re.sub("\D", "", FilterDigits) )

                    

            return True


    except Exception as e:
       SendInfoToDisplay(e)
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
       SendInfoToDisplay(e)
       return False
    return False



def FindWlanToUseGround():
    global WlanName

    PrimaryCardMAC = GetPrimaryCardMAC_Config()
    if PrimaryCardMAC == False or PrimaryCardMAC == "0":
        SendInfoToDisplay("Trying to init WLAN...")
        try:
            for root, dirs, files in os.walk("/sys/class/net/"):
                for dir in dirs:
                    if dir.startswith("eth") == False and  dir.startswith("lo") == False and  dir.startswith("usb") == False and  dir.startswith("intwifi") == False and  dir.startswith("relay") == False and dir.startswith("wifihotspot") == False:
                        SendInfoToDisplay("Found WLan with name: " + dir)
                        WlanName = dir
            if WlanName != "0":
                SendInfoToDisplay("Using WLAN with name: " + WlanName)
                subprocess.check_call(['/sbin/iw', "dev", WlanName , "set", "freq", "2412" ])
                SendInfoToDisplay("Frequency set to 2412")
                return True
            else:
                return False

        except Exception as e:
            SendInfoToDisplay(e)
            return False
    else:
        SendInfoToDisplay("Try to find wlan with MAC:" + PrimaryCardMAC)
        result = FindWlanNameByMAC(PrimaryCardMAC)
        if result != False:
            SendInfoToDisplay("Wlan with MAC found ")
            try:
                WlanName = result
                SendInfoToDisplay("Using WLAN with name: " + WlanName)
                subprocess.check_call(['/sbin/iw', "dev", WlanName , "set", "freq", "2412" ])
                SendInfoToDisplay("Frequency set to 2412")
                return True
            except Exception as e:
                SendInfoToDisplay(e)
                return False
        else:
            SendInfoToDisplay("Wlan with MAC" + PrimaryCardMAC +  "not found, looking any other...")
            try:
                for root, dirs, files in os.walk("/sys/class/net/"):
                    for dir in dirs:
                        if dir.startswith("eth") == False and  dir.startswith("lo") == False and  dir.startswith("usb") == False and  dir.startswith("intwifi") == False and  dir.startswith("relay") == False and dir.startswith("wifihotspot") == False:
                            SendInfoToDisplay("Found WLan with name: " + dir)
                            WlanName = dir
                if WlanName != "0":
                    SendInfoToDisplay("Using WLAN with name: "  + WlanName)
                    subprocess.check_call(['/sbin/iw', "dev", WlanName , "set", "freq", "2412" ])
                    SendInfoToDisplay("Frequency set to 2412")
                    return True
                else:
                    return False

            except Exception as e:
                SendInfoToDisplay(e)
                return False

#Not used. Can be used with ath9k driver reload.
def InitWlan():
    global WlanName
    PrimaryCardMAC = GetPrimaryCardMAC_Config()
    if PrimaryCardMAC == False or PrimaryCardMAC == "0":
        SendInfoToDisplay("Trying to init wlan0...")
        try:
            if os.path.isdir("/sys/class/net/wlan0") == True:
                WlanName = "wlan0"
                subprocess.check_call(['/home/pi/RemoteSettings/Ground/SetWlanXMonitorModeFreq.sh', "wlan0" ,"2412" ])
                return True
            else:
                SendInfoToDisplay("wlan0 not found")
                return False
        except Exception as e:
            SendInfoToDisplay(e)
            return False
    else:
        SendInfoToDisplay("Try to configure wlan with MAC:" + PrimaryCardMAC)
        result = FindWlanNameByMAC(PrimaryCardMAC)
        if result != False:
            SendInfoToDisplay("Wlan name with MAC: " + result)
            try:
                WlanName = result
                subprocess.check_call(['/home/pi/RemoteSettings/Ground/SetWlanXMonitorModeFreq.sh', result ,"2412" ])
                return True
            except Exception as e:
                SendInfoToDisplay(e)
                return False
        else:
            SendInfoToDisplay("Wlan with MAC not found, trying wlan0...")
            try:
                if os.path.isdir("/sys/class/net/wlan0") == True:
                    WlanName = "wlan0"
                    subprocess.check_call(['/home/pi/RemoteSettings/Ground/SetWlanXMonitorModeFreq.sh', "wlan0" ,"2412" ])
                    return True
                else:
                    SendInfoToDisplay("wlan0 not found")
                    return False
            except Exception as e:
                SendInfoToDisplay(e)
                return False

def ShutDownWlan():
    try:
        subprocess.check_call(['/bin/ip', "link" ,"set", WlanName, "down" ])
        return True
    except Exception as e:
        SendInfoToDisplay(e)
        return False
    return False

def IsWlanAth9k():
    try:
        fPath = "/sys/class/net/" + WlanName + "/device/uevent"
        with open(fPath, "r") as f:
            lines = f.readlines()
            for line in lines:
                if "ath9k_htc" in line:
                    SendInfoToDisplay("Wlan is ath9k_htc. Driver reload required")
                    return True
        return False
    except Exception as e:
        SendInfoToDisplay(e)
        return False
    return False


def UnloadAth9kDriver():
    SendInfoToDisplay("Unload Ath9k_htc Driver...")
    try:
        subprocess.check_call(['rmmod', "ath9k_htc"  ])
        subprocess.check_call(['rmmod', "ath9k_common"  ])
        subprocess.check_call(['rmmod', "ath9k_hw"  ])
        subprocess.check_call(['rmmod', "ath"  ])
        return True
    except Exception as e:
        SendInfoToDisplay(e)
        return False
    return False

def LoadAth9kDriver():
    SendInfoToDisplay("Load Ath9k_htc Driver...")
    try:
        subprocess.check_call(['modprobe', "ath9k_htc"  ])
        subprocess.check_call(['modprobe', "ath9k_common"  ])
        subprocess.check_call(['modprobe', "ath9k_hw"  ])
        subprocess.check_call(['modprobe', "ath"  ])
        return True
    except Exception as e:
        SendInfoToDisplay(e)
        return False
    return False

def CreateFinishMarkFile():
    try:
        f = open("/tmp/ReadyToGo", "w")
        f.write("1")
        f.close()
        return True
    except Exception as e:
        SendInfoToDisplay(e)
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
        SendInfoToDisplay(e)
    return False

def StartSVPcomTx():                                     
    try:                                     
        subprocess.Popen(['/home/pi/cameracontrol/IPCamera/svpcom_wifibroadcast/wfb_tx',"-k", "1", "-n", "1",
                          "-K", "/home/pi/cameracontrol/IPCamera/svpcom_wifibroadcast/tx.key",
                          "-u" ,str(UDP_PORT_OUT), "-p", "93", "-B", "20", "-M", "0", WlanName ])
        return True
    except Exception as e:
        SendInfoToDisplay(e)
        return False
    return False

def StartSVPcomRx():   
    try:                                     
        subprocess.Popen( ['/home/pi/cameracontrol/IPCamera/svpcom_wifibroadcast/wfb_rx', "-k", "1", "-n", "1",
                               "-K", "/home/pi/cameracontrol/IPCamera/svpcom_wifibroadcast/rx.key",
                              "-c" ,"127.0.0.1", "-u", str(UDP_PORT_IN), "-p", "92",  WlanName ], stdout=RxDevNull)
        return True
    except Exception as e:
        SendInfoToDisplay(e)
        return False
    return False

def StartRC_Reader(ChannelToRead):   
    try:
        if ChannelToRead > 8:
            SendInfoToDisplay("Selected RC channel greater than 8.  Forced to channel 1")
            ChannelToRead = 1
        if ChannelToRead < 1:
           SendInfoToDisplay("Selected RC channel less than 1.  Forced to channel 1")
           ChannelToRead = 1

        subprocess.Popen( ['/home/pi/RemoteSettings/Ground/helper/JoystickSender', str(ChannelToRead)], stdout=RCDevNull)
        return True
    except Exception as e:
        SendInfoToDisplay(e)
        return False
    return False

def IsTimeToExitByTimer():
    if StartTime != 0:
        TimeNow = datetime.now()
        diff = (TimeNow - StartTime).total_seconds()
        if diff > 5:
            if diff > SettingsSyncOffByTimer:
                if LastRequestTime != 0:
                    diffLastRequest = (TimeNow - LastRequestTime).total_seconds()
                    if NotBreakByTimerIfLastRequestWas > diffLastRequest:
                        SendInfoToDisplay("Sync in progress.... Timer - delayed.")
                        return False
                    
                SendInfoToDisplay("Program interrupted by RC timer")
                return True
    return False

def IsTimeToExitByTimeout():
    if LastRequestTime != 0:
        TimeNow = datetime.now()
        diffLastRequest = (TimeNow - LastRequestTime).total_seconds()
        if diffLastRequest > NotBreakByTimerIfLastRequestWas:
            SendInfoToDisplay("Timeout.")
            return True
    return False


def IsTimeToExit():
    global ExitRCThread

    if RC_Value >= SettingsSyncForceOffRC_Value and RC_Value != 0:
        SendInfoToDisplay("Program interrupted by RC joystick")
        ExitRCThread = 1
        return True
    else:
        return False

def InitUDPServer():
    for i in range(0,10):
        SendInfoToDisplay("SwitchToAwaiting")
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
        SendInfoToDisplay(e)
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
                SendInfoToDisplay("RequestGroundChecksum")
                LastRequestTime = datetime.now()
                result = md5(SettingsFilePath)
                if result != False:

                    md5Bytes = result.encode('ascii')
                    SendInfoToDisplay("md5: " + str(md5Bytes ) )
                    SizeInBytesInt = os.path.getsize(SettingsFilePath)
                    SendInfoToDisplay("File size in bytes:" + str(SizeInBytesInt) )
                    if SizeInBytesInt != 0:
                        MessageBufFile.extend(md5Bytes)
                        SizeInBytesString = str(SizeInBytesInt)
                        SizeInBytesByte = SizeInBytesString.encode('ascii')
                        MessageBufFile.extend(SizeInBytesByte)
                        SendData(MessageBufFile)
                        MessageBufFile.clear()
            if RequestSFile in data:
                SendInfoToDisplay("SwitchToDownload")
                LastRequestTime = datetime.now()
                #return: 6 bytes - offset that was read + data 1024 bytes or less
                if len(data) > 13:
                    offset = data[13:len(data)]
                    SendInfoToDisplay("Requested file from offset: " + str(offset ) )
                    offsetInt = int(offset)
                    SendInfoToDisplay("OffsetInt: " + str(offsetInt) )

                    SizeInBytes = os.path.getsize(SettingsFilePath)
                    BytesTillEndOfFile = SizeInBytes - offsetInt

                    headerStr =  '{:0>6}'.format(offsetInt)
                    header = headerStr.encode('ascii')
                    if BytesTillEndOfFile >= 1024:
                        MessageBufFile.extend(header)
                        SendInfoToDisplay("Till end of file left more than 1024")
                        tmp = ReadFileFrom(offsetInt,1024)
                        SendInfoToDisplay("Bytes read From file:" + str(len(tmp)) )
                        MessageBufFile.extend(tmp)
                        SendInfoToDisplay("Total MessageBuf len: " + str( len(MessageBufFile) ))
                        SendData(MessageBufFile)
                        MessageBufFile.clear()
                    else:
                        MessageBufFile.extend(header)
                        SendInfoToDisplay("Till end of file left: " + str(BytesTillEndOfFile) )
                        MessageBufFile.extend(ReadFileFrom(offsetInt,BytesTillEndOfFile))
                        SendData(MessageBufFile)
                        MessageBufFile.clear()

            if data == DownloadFinished:
                LastRequestTime = datetime.now()
                SendInfoToDisplay("DownloadFinished")
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
                SendInfoToDisplay("Starting main OpenHD shell script.")
                CleanAndExit()
                exit()
                

            if data == NoNeedInSync:
                LastRequestTime = datetime.now()
                SendInfoToDisplay("NoNeedInSync")
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
                SendInfoToDisplay("Starting main OpenHD shell script.")
                CleanAndExit()
                exit()


        except Exception as e:
            pass
            #SendInfoToDisplay(e)

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
                SendInfoToDisplay("RC thread exiting...")
                break
        except:
            if ExitRCThread == 1:
                SendInfoToDisplay("RC thread exiting...")
                break

    RCSock.close()
    SendInfoToDisplay("RC thread terminated")


def ReturnWlanFreq():
    if FreqFromConfigFile != "0" and WlanName != "0":
        try:
            subprocess.check_call(['/home/pi/RemoteSettings/Ground/SetWlanFreq.sh', WlanName , FreqFromConfigFile ])
            SendInfoToDisplay("Frequency for WLAN: " + WlanName + " returned back to: " + FreqFromConfigFile)
        except Exception as e:
            SendInfoToDisplay(e)

def CleanAndExit():
    global ExitRCThread
    global RxDevNull
    global RCDevNull

    SendInfoToDisplay("SettingsSync done.")
    ExitRCThread = 1
    ReturnWlanFreq()
    sleep(1)

    try:
        subprocess.check_call(['/usr/bin/killall', "JoystickSender" ]) 
    except Exception as e:
        SendInfoToDisplay(e)

    try:
        subprocess.check_call(['/usr/bin/killall', "wfb_rx" ]) 
    except Exception as e:
        SendInfoToDisplay(e)

    try:
        subprocess.check_call(['/usr/bin/killall', "wfb_tx" ]) 
    except Exception as e:
        SendInfoToDisplay(e)

    
    try:
        subprocess.check_call(['/usr/bin/killall', "DisplayProgram" ]) 
    except Exception as e:
        print(e)

    

    RxDevNull.close()
    RCDevNull.close()
    exit()

def ShowSettings():
    SendInfoToDisplay(" ")
    SendInfoToDisplay("SettingsSyncForceOffRC_Value=" + str(SettingsSyncForceOffRC_Value ) )
    SendInfoToDisplay("SettingsSyncForceOnRC_Value=" + str(SettingsSyncForceOnRC_Value) )
    SendInfoToDisplay("SettingsSyncRC_Channel=" + str(SettingsSyncRC_Channel) )
    SendInfoToDisplay("SettingsSyncStartOption=" + str(SettingsSyncStartOption) )
    SendInfoToDisplay("SettingsSyncOffByTimer=" + str(SettingsSyncOffByTimer) )
    SendInfoToDisplay("NotBreakByTimerIfLastRequestWas=" + str(NotBreakByTimerIfLastRequestWas) )


#################################################### start


if os.path.isfile("/tmp/ReadyToGo") == True:
    SendInfoToDisplay("No need to run second time")
    exit()

InitDevNull()

RC_UDP_IN_thread = threading.Thread(target=StartRCThreadIn)
RC_UDP_IN_thread.start()

SendInfoToDisplay("Parse config file...")
if ReadSettingsFromConfigFile() == True:
    SendInfoToDisplay("Completed without errors")
    ShowSettings()
else:
    SendInfoToDisplay("Completed with errors. Using default settings. Check ground config file.")
    SettingsSyncForceOffRC_Value=1700
    SettingsSyncForceOnRC_Value=1400
    SettingsSyncRC_Channel=1
    SettingsSyncStartOption=1
    SettingsSyncOffByTimer=20
    NotBreakByTimerIfLastRequestWas=3
    ShowSettings()

StartRC_Reader(SettingsSyncRC_Channel)

if SettingsSyncStartOption != 1:
    SendInfoToDisplay("SettingsSync disabled.Starting RC reader to check force On ")
    for i in range(0, 10):
        #SendInfoToDisplay("I is:", i, "RC value: ", RC_Value)
        #stdout.write("\r RC value: "+  str(RC_Value) + " Retry: " + str(i) + " of 30")
        #stdout.flush()
        SendInfoToDisplay(" RC value: "+  str(RC_Value) + " Retry: " + str(i) + " of 10")
        if RC_Value <= SettingsSyncForceOnRC_Value and RC_Value != 0:
            SettingsSyncStartOption = 1
            SettingsSyncOffByTimer=0
            SendInfoToDisplay("SettingsSync forced to On via joystick")
            SendInfoToDisplay("Timer disabled")
            break
        if RC_Value >= SettingsSyncForceOffRC_Value and RC_Value != 0:
            SettingsSyncStartOption = 0
            SendInfoToDisplay("SettingsSync forced to Off via joystick")
            break

        sleep(0.3)


if SettingsSyncStartOption == 1:
    SendInfoToDisplay("SettingsSync init...")
    #if InitWlan() != False:
    if FindWlanToUseGround() != False:
        if StartSVPcomRx() != False:
            if StartSVPcomTx() != False:
                if SettingsSyncOffByTimer > 5:
                    SendInfoToDisplay("Starting timer...")
                    StartTime = datetime.now()
                else:
                    SendInfoToDisplay("Timer set to less than 5 seconds. Disabled.")

                InitUDPServer()
            else:
                SendInfoToDisplay("Can`t init radio TX")
        else:
            SendInfoToDisplay("Can`t init radio RX")
    
    else:
        SendInfoToDisplay("Can`t init Wlan. Exit.")
else:
    SendInfoToDisplay("SettingsSync disabled.")

CleanAndExit()
