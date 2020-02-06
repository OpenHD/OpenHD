import logging
from logging.handlers import RotatingFileHandler

log = logging.getLogger()
if log.handlers:
    for handler in log.handlers:
        log.removeHandler(handler)

log_formatter = logging.Formatter('[%(asctime)s] {%(funcName)-15s:%(lineno)d} %(levelname)-8s - %(message)s','%m-%d %H:%M:%S')
console = logging.StreamHandler()
console.setFormatter(log_formatter)

app_log = logging.getLogger('generic')
log.setLevel(logging.DEBUG)
log.addHandler(console)



import fileinput
import socket
import hashlib
import os
from time import sleep
import RPi.GPIO as GPIO
import subprocess
import re

UDP_PORT_OUT = 1375
UDP_PORT_IN = 1376
RecvSocket = 0
RetryCountMD5 = 15
FileSizeInt = 0
SettingsFilePath = "/boot/openhd-settings-1.txt"
SwitchToFreq = "0"
DefaultCommunicateFreq = "2412"

SettingsFilePath = "/boot/openhd-settings-1.txt"
TxPowerConfigFilePath="/etc/modprobe.d/ath9k_hw.conf"
USER_SETTINGS = {}
USER_SETTINGS['TxPowerGround']="-1"
USER_SETTINGS['TxPowerFromAth9k_hw']="-1"
#USER_SETTINGS['TxPowerFromAth9k_hw']="-1"
USER_SETTINGS['WlanName'] = "0"
USER_SETTINGS['DATARATE'] = "0"
USER_SETTINGS['TXPOWER']= "0"
USER_SETTINGS['TXMODE'] = "0"
#USER_SETTINGS['SmartSync_StartupMode'] = "-1"


GPIO.setmode(GPIO.BCM)

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


def run_bash(stringa):
    devnull = open('/dev/null', 'w')
    try:
        subprocess.Popen(stringa, stdout=devnull,shell=True)
        log.debug("runnig bash: {}".format(stringa))
        return True
    except Exception as e:
        log.error(e)
    return False


def SendData(MessageBuf):
    try:
        sockToAir = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
        sockToAir.sendto( bytes(MessageBuf,'utf-8'), ('127.0.0.1', UDP_PORT_OUT))
    except Exception as e:
        log.error(e)
        return False

def InitUDPServer():
    global RecvSocket
    UDP_IP = ""
    try:
        RecvSocket = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
        RecvSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        RecvSocket.settimeout(0.3)
        RecvSocket.bind((UDP_IP, UDP_PORT_IN))
    except Exception as e:
        log.error(e)
        return False

def RecvPacket():
    try:
            data, addr = RecvSocket.recvfrom(1200)
            log.debug(str(data))
            return data
    except Exception as e:
        log.error(str(e))
        return False



def RequestSettingsFile():
    #recv packet header: 6 bytes offset + data up to 1024. Max 1030 bytes
    InBuffer = ""
    offset = 0
    isReceived = False
    MessageBufFile =  bytearray()

    while isReceived == False:
        try:
            if offset < FileSizeInt:
                SendBuff = "RequestSFile"
                SendBuff +=  '{:0>6}'.format(offset)
                SendData(SendBuff)
                tmp = RecvPacket()
                if tmp != False:
                    InMsgLen = len(tmp)
                    RecvPacketOffset = int(tmp[0:6])
                    if RecvPacketOffset == offset:
                        MessageBufFile.extend(tmp[6:InMsgLen])
                        offset += InMsgLen-6
                        log.debug("File block received, offset", offset)
                    else:
                        log.debug("Wrong packet, ignore")
                else:
                    log.debug("Recv file block error. Retry...")
                sleep(0.05)
            else:
                isReceived = True
        except Exception as e:
            log.debug(e)
    return MessageBufFile

def SaveFile(Buf, path):
    try:
        hfile = open(path, "wb")
        hfile.write(Buf)
        hfile.close()
        return True
    except Exception as e:
            log.debug(e)
            return False
    return True

def md5(fname):
    try:
        hash_md5 = hashlib.md5()
        with open(fname, "rb") as f:
            for chunk in iter(lambda: f.read(4096), b""):
                hash_md5.update(chunk)
        return hash_md5.hexdigest().encode('ascii')
    except Exception as e:
       log.debug(e)
       return False

def RequestMd5FileSize():
    global FileSizeInt
    FailCounter = 0

    for FailCounter in range(0, RetryCountMD5):
        SendData("RequestGroundChecksum")
        result = RecvPacket()
        if result != False:
            StrLen = len(result)
            if StrLen > 33: #32 bytes md5, after 32byte - file size
                #Get MD5CheckSum
                MD5CheckSum = result[0:32]
                log.debug("Ground settings file MD5:{}".format(MD5CheckSum))
                #Get FileSize
                FileSizeStr = result[32:StrLen]
                FileSizeInt = int(FileSizeStr)
                log.debug("Settings file size in bytes:{}".format(FileSizeInt))
                return MD5CheckSum

        else:
            FailCounter+= 1
            log.debug("RequestMd5FileSize error. Retry: {}".format(FailCounter) )
    return False

def NotifyGroundWithACK(message):
    SendData(message)
    result = RecvPacket()
    if result != False:
        tmp = "ACK".encode('ascii')
        if result == tmp:
            return True
        else:
            return False
    return False

def MoveFile():
    os.system('mount -o remount,rw /boot')
    os.system('cp /tmp/infile.txt /boot/openhd-settings-1.txt')
    os.system('sync')
    os.system('mount -o remount,ro /boot')
    log.debug("copy file process completed.")


def StartSVPcomTx():
    return run_bash('/home/pi/cameracontrol/IPCamera/svpcom_wifibroadcast/wfb_tx -k 1 -n 1 -K /home/pi/cameracontrol/IPCamera/svpcom_wifibroadcast/tx.key -u {UDP_PORT_OUT} -p 92 -B 20 -M 0 {interface}'.format(
        UDP_PORT_OUT=UDP_PORT_OUT,
        interface=USER_SETTINGS['WlanName'])
        )

def StartSVPcomRx():
    return run_bash('/home/pi/cameracontrol/IPCamera/svpcom_wifibroadcast/wfb_rx -k 1 -n 1 -K /home/pi/cameracontrol/IPCamera/svpcom_wifibroadcast/rx.key -c {IP_ADRRESS} -u {UDP_PORT_OUT} -p 93  {interface}'.format(
        IP_ADRRESS='127.0.0.1',
        UDP_PORT_OUT=UDP_PORT_OUT,
        interface=USER_SETTINGS['WlanName'])
        )

def StartConfigureWlanScript():
    return run_bash('/home/pi/RemoteSettings/Air/helper/ConfigureNicsAir.sh {DATARATE} {FREQ} single'.format(
        DATARATE=USER_SETTINGS['DATARATE'],
        FREQ=DefaultCommunicateFreq)
        )


def FindWlanToUseAir():
    log.debug("Trying to init WLAN...")
    try:
        for root, dirs, files in os.walk("/sys/class/net/"):
            for dir in dirs:
                if dir.startswith("eth") == False and  dir.startswith("lo") == False and  dir.startswith("usb") == False and  dir.startswith("intwifi") == False and  dir.startswith("relay") == False and dir.startswith("wifihotspot") == False:
                    log.debug("Found WLan with name: {}".format(dir))
                    USER_SETTINGS['WlanName'] = dir
        if USER_SETTINGS['WlanName'] != "0":
            log.debug("Using WLAN with name: {}".format( USER_SETTINGS['WlanName']))
            subprocess.check_call(['/sbin/iw', "dev", USER_SETTINGS['WlanName'] , "set", "freq", DefaultCommunicateFreq ])
            return True
        else:
            return False

    except Exception as e:
        log.error(e)
        return False

def ReturnWlanFreq():
    if USER_SETTINGS['WlanName'] != "0":
        try:
            if USER_SETTINGS['FREQ'] == "0":
                #subprocess.check_call(['/sbin/iw', "dev", USER_SETTINGS['WlanName'] , "set", "freq", "2472" ])
                subprocess.check_call(['/home/pi/RemoteSettings/Air/SetWlanFreq.sh', USER_SETTINGS['WlanName'] , "2472" ])
                log.debug("Can`t read frequency from config file. Frequency set to: 2472")
            else:
                #subprocess.check_call(['/sbin/iw', "dev", USER_SETTINGS['WlanName'] , "set", "freq", USER_SETTINGS['FREQ'] ])
                subprocess.check_call(['/home/pi/RemoteSettings/Air/SetWlanFreq.sh', USER_SETTINGS['WlanName'] , USER_SETTINGS['FREQ'] ])
                log.debug("Frequency for WLAN: " + USER_SETTINGS['WlanName'] + " returned back to: " + USER_SETTINGS['FREQ'])


        except Exception as e:
            log.debug(str(e))

def ReadSettingsFromConfigFile():
    try:
        with open(SettingsFilePath, 'r') as document:
            for line in document:
                if (not line.startswith('#') and not line.startswith('\n')):
                    line = line.split('=')
                    if not line:
                        continue
                    USER_SETTINGS[line[0]] = str(line[1]).replace('\n', '')
            return True
    except Exception as e:
       log.error(e)
       return False
    return False


def InitSettings():
    ReadSettingsFromConfigFile()

    if USER_SETTINGS['DATARATE'] == "0":
        log.debug("Can't read DATARATE. Set DATARATE=4")
        USER_SETTINGS['DATARATE'] = "4"

    if USER_SETTINGS['TXPOWER']== "0":
        USER_SETTINGS['TXPOWER']= "2000"

    if USER_SETTINGS['TXMODE'] == "0":
        USER_SETTINGS['TXMODE'] = "single"

def ReadTxPowerAth9k_hw():
    ####           if line.startswith("options ath9k_hw txpower") == True:
    return True if 'TxPowerFromAth9k_hw' in USER_SETTINGS else False

def ReadTxPower():
    return True if 'TxPowerGround' in USER_SETTINGS else False

def CheckTxPower():
    try:
        if ReadTxPowerAth9k_hw() != False:
            log.debug("USER_SETTINGS['TxPowerFromAth9k_hw']= " + USER_SETTINGS['TxPowerFromAth9k_hw'])
            if ReadTxPower() != False:
                log.debug("USER_SETTINGS['TxPowerGround']= " + USER_SETTINGS['TxPowerGround'])
                if USER_SETTINGS['TxPowerGround'] != USER_SETTINGS['TxPowerFromAth9k_hw']:
                    log.debug("TxPower not equal Check if all ok and apply")
                    if USER_SETTINGS['TxPowerFromAth9k_hw'] != "-1" and USER_SETTINGS['TxPowerGround'] != "-1":
                        log.debug("all ok, apply")
                        subprocess.check_call(['/usr/local/bin/txpower_atheros', USER_SETTINGS['TxPowerGround'] ] )
                        return True
    except Exception as e:
        log.error(e)
        return False
    return False


def CleanAndExit():
    CheckTxPower()
    log.debug("SmartSync done.")
    ReturnWlanFreq()
    sleep(1)
    RecvSocket.close()

    try:
        subprocess.check_call(['/usr/bin/killall', "JoystickSender" ])
    except Exception as e:
        log.error(e)

    try:
        subprocess.check_call(['/usr/bin/killall', "wfb_rx" ])
    except Exception as e:
        log.error(e)

    try:
        subprocess.check_call(['/usr/bin/killall', "wfb_tx" ])
    except Exception as e:
        log.error(e)


    exit()

#########################################################Start
if __name__ == '__main__':

    if os.path.isfile("/tmp/ReadyToGo") == True:
        log.debug("No need to run second time")
        exit()

    InitSettings()

    if StartConfigureWlanScript() != False:
        if FindWlanToUseAir() != False:
            if StartSVPcomRx() != False:
                log.debug("StartSVPcomRx")
                if StartSVPcomTx() != False:
                    log.debug("StartSVPcomTx")
                    InitUDPServer()
                    MD5CheckSumGround = RequestMd5FileSize()
                    if MD5CheckSumGround != False:
                        MD5CheckSumAirCurrent = md5(SettingsFilePath)
                        if MD5CheckSumGround == MD5CheckSumAirCurrent:
                            log.info("Air and Ground config files equal. No need in sync")
                            log.info("Notify ground that it can boot.")
                            IsACK_RetryCounter = 0
                            for i in range(0,15):
                                IsACK = NotifyGroundWithACK("NoNeedInSync")
                                if IsACK == True:
                                    CleanAndExit()
                                    sleep(0.1)
                                    CleanAndExit()
                                else:
                                    log.error("Air and Ground config mismatch. Sync required")
                                    while True:
                                        InFileBuff = RequestSettingsFile()
                                        SaveFile(InFileBuff,"/tmp/infile.txt")
                                        InFileHash = md5("/tmp/infile.txt")
                                        log.debug("InFileMD5: {}".format(InFileHash))
                                        if InFileHash == MD5CheckSumGround:
                                            log.debug("Ground and downloaded file checksum equal.")
                                            log.debug("ACK received. Moving tmp file to /boot...")
                                            MoveFile()
                                            for x in range(0,15):
                                                IsACK = NotifyGroundWithACK("DownloadFinished")
                                                if IsACK == True:
                                                    log.debug("ACK received. ready to boot")
                                                    CleanAndExit()
                                                    break
                                                    CleanAndExit()

                                                else:
                                                    log.debug("Downloaded file checksum not match to Ground. Retry...")


                                            else:
                                                log.debug("Failed to request ground MD5 config checksum. Current config file will be loaded.")
                                        else:
                                            log.debug("Faile to start /home/pi/RemoteSettings/Air/helper/ConfigureNicsAir.sh file")

                                            CleanAndExit()
