import subprocess
from subprocess import call
from tempfile import mkstemp
from shutil import move
from os import fdopen, remove


TXPOWER_RAILINK='TxPower_R'
TXPOWER_ATHEROS='TxPower_A'

def read_txpower_settings(response_header):
    txp = {}

    processA = subprocess.Popen(["sed", "s/.*txpower=\\([0-9]*\\).*/\\1/", "/etc/modprobe.d/ath9k_hw.conf"], stdout=subprocess.PIPE)
    ret_txpowerA = processA.communicate()[0]
    txpowerA = re.sub("[^0-9]+", "", ret_txpowerA.decode('utf-8'))
    processR = subprocess.Popen(["sed", "s/.*txpower=\\([0-9]*\\).*/\\1/", "/etc/modprobe.d/rt2800usb.conf"], stdout=subprocess.PIPE)
    ret_txpowerR = processR.communicate()[0]
    txpowerR = re.sub("[^0-9]+", "", ret_txpowerR.decode('utf-8'))

    txp['txpowerR'] = txpowerR
    txp['txpowerA'] = txpowerA

    print("txpowerR: ")
    print(txp['txpowerR'])
    print("txpowerA: ")
    print(txp['txpowerA'])

    response_header['settings'].update(txp)
    return response_header

def replace_TxPower_config(Param, Value):

    if 'txpowerA' in Param:
        subprocess.Popen(["txpower_atheros", Value])

    if 'txpowerR' in Param:
        subprocess.Popen(["txpower_ralink", Value])

    return 1