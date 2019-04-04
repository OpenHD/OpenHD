
#listen for messages coming via UDP
#(forwarded by the ground pi)
#change value and send acknowledging response
#NOTE: Connecting to the air pi directly with the settings app is not supported !


import socket
import sys
import io
from Message import ParseMessage,BuildMessageCHANGE_OK,BuildMessageGET_OK,BuildMessageHELLO_OK
from SettingsDatabase import changeSetting,createSettingsDatabase,getValueForKey
from Forwarder import SendMessageToGroundPi

DEBUG_ME=None
settingsDatabase=None


#change value from x to y
#send response to the Ground PI
def processChangeMessageLocally(key,value):
    print("Changing Key on air pi:",key,"Value:",value)
    global settingsDatabase
    changeSetting(settingsDatabase,key,value)
    #refresh the local database
    settingsDatabase=createSettingsDatabase(DEBUG_ME)
    return BuildMessageCHANGE_OK("A",key,value)   

#optain value for key
#send response to the ground pi
def processGetMessageLocally(key):
    print("Optaining value for Key on air pi:",key)
    value=getValueForKey(settingsDatabase,key)
    if(value==None):
        value="INVALID_SETTING"
    return BuildMessageGET_OK("A",key,value)

def processMessageLocally(cmd,data):
    if(cmd=="HELLO"):
        return BuildMessageHELLO_OK("A")
    if(cmd=="CHANGE"):
        key,value=data.split("=")
        return processChangeMessageLocally(key,value)
    elif(cmd=="GET"):
        return processGetMessageLocally(data)

#process messages coming from the ground pi transmitted by the lossy EZ-WB connection
def parseMessageFromGroundPi(msg):
    print("message on air",msg)
    dst,cmd,data=ParseMessage(msg)
    if(dst=='A' or dst=='GA'):
        return processMessageLocally(cmd,data)
    else:
        print("Wrong id at air pi:",dst)


#This one has to run on the Air Pi continiously for the settings app to work
def ReplyLoop():
    global settingsDatabase
    settingsDatabase=createSettingsDatabase(DEBUG_ME)
    receiveSock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    receiveSock.bind(('localhost',9090))
    print("started reply loop on air pi")
    while True:
        data=receiveSock.recv(1024)
        #here we don't parse into lines, but assume that when receiving data it is exactly one line
        response=parseMessageFromGroundPi(data.decode())
        if response:
            SendMessageToGroundPi(response)
        

ReplyLoop()
