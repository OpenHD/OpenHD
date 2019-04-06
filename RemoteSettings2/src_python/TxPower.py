import subprocess
from subprocess import call
from tempfile import mkstemp
from shutil import move
from os import fdopen, remove
import re

TXPOWER_ATHEROS='TxPower_A'
TXPOWER_RAILINK='TxPower_R'

def read_txpower_settings():
    txp = {}
    try:
        processA = subprocess.Popen(["sed", "s/.*txpower=\\([0-9]*\\).*/\\1/", "/etc/modprobe.d/ath9k_hw.conf"], stdout=subprocess.PIPE)
        ret_txpowerA = processA.communicate()[0]
        txpowerA = re.sub("[^0-9]+", "", ret_txpowerA.decode('utf-8'))
        processR = subprocess.Popen(["sed", "s/.*txpower=\\([0-9]*\\).*/\\1/", "/etc/modprobe.d/rt2800usb.conf"], stdout=subprocess.PIPE)
        ret_txpowerR = processR.communicate()[0]
        txpowerR = re.sub("[^0-9]+", "", ret_txpowerR.decode('utf-8'))
    except Exception as e:
        txpowerA='E'
        txpowerR='E'
        print("TX Power error read",e)

    txp.update({TXPOWER_ATHEROS : txpowerA})
    txp.update({TXPOWER_RAILINK : txpowerR})
	
    #print(TXPOWER_ATHEROS,ret_txpowerA)
    #print(TXPOWER_RAILINK,ret_txpowerR)
    print(txp)
    return txp


def replace_txpower_setting(Param, Value):
    try:
        if Param == TXPOWER_ATHEROS:
            print("replacing",Param,Value)
            subprocess.Popen(["txpower_atheros", Value])
        if Param == TXPOWER_RAILINK:
            print("replacing",Param,Value)
            subprocess.Popen(["txpower_ralink", Value])
    except Exception as e:
        print("TX power error replace",e)
	