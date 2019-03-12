
#listen for messages coming via UDP
#(forwarded by the ground pi)
#change value and send acknowledging response
#NOTE: Connecting to the air pi directly with the settings app is not supported !


import socket
import sys
import io
from Message import ParseMessage,BuildMessageCHANGE_OK,BuildMessageGET_OK
from SettingsDatabase import changeSetting,createSettingsDatabase,getValueForKey

settingsDatabase=None

#change value from x to y
#send response to the Ground PI
def processChangeMessage(key,value):
    print("Changing Key on air pi:",key,"Value:",value)
    global settingsDatabase
    changeSetting(settingsDatabase,key,value)
    #refresh the local database
    settingsDatabase=createSettingsDatabase(None)
    return BuildMessageCHANGE_OK("A",key,value)   


#optain value for key
#send response to the ground pi
def processGetMessage(key):
    print("Optaining value for Key on air pi:",key)
    value=getValueForKey(settingsDatabase,key)
    if(value==None):
        value="INVALID_SETTING"
    return BuildMessageGET_OK("A",key,value)


#process messages coming from the ground pi transmitted by the lossy EZ-WB connection
def processMessageFromGroundPi(msg):
    print("message on air",msg)
    cmd,data=ParseMessage(msg)
    if(cmd=="CHANGE"):
        key,value=data.split("=")
        return processChangeMessage(key,value)
    elif(cmd=="GET"):
        return processGetMessage(data)

#only call this one from the Reply loop context
def sendMessageToGroundPi(message):
    sendSock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    sendSock.sendto((message).encode(),('localhost', 9292))
    print("Sent to ground pi:",message)


#This one has to run on the Air Pi continiously for the settings app to work
def ReplyLoop():
    global settingsDatabase
    settingsDatabase=createSettingsDatabase('A')
    receiveSock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    receiveSock.bind(('localhost',9090))
    print("started reply loop on air pi")
    while True:
        data=receiveSock.recv(1024)
        #here we don't parse into lines, but assume that when receiving data it is exactly one line
        #if(data):
        response=processMessageFromGroundPi(data.decode())
        sendMessageToGroundPi(response)
        

ReplyLoop()