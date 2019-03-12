
#this server runs on the ground pi
#It shall be the main entry point for external devices to connect to the Open.HD system
#Currently it only processes 'CHANGE and GET SETTINGS' request coming from the Android Settings module,but it 
#should not be limited to only that - e.g. starting/stopping an rtsp server for video over TCP (not udp)
#could be easily implemented here by adding more supported messages

import socket
import sys
from io import StringIO
import io
from SettingsDatabase import *
import time
import threading
from threading import Thread
from Forwarder import ForwardMessageToAirPiAndAwaitResponse,ForwardMessageToAirPi
from Message import *
from time import sleep
from queue import Queue


#Thread-safe queue for messages that should be sent to the client. Read by the TCP thread, written by (e.g) the WFB receiver thread or the TCP thread itself
messagesForClient=Queue()
#messagesForClient.put("Hello from queue")
settingsDatabase=createSettingsDatabase(None)
lastHELLO_OKmessage=0.0

#Change value on ground pi
#forward message to air pi
def processChangeMessage(key,value):
    print("Changing Key on ground pi:",key,"Value:",value)
    global settingsDatabase
    #Change value from x to y on ground pi
    changeSetting(settingsDatabase,key,value)
    #refresh the local database
    settingsDatabase=createSettingsDatabase('G')
    #forward message to air pi, we will receive the response in a different Thread
    ForwardMessageToAirPi(BuildMessageCHANGE(key,value))
    global messagesForClient
    messagesForClient.put(BuildMessageCHANGE_OK("G",key,value))


#return value on ground pi
#forward message to air pi
def processGetMessage(key):
    print("Optaining value for Key on ground pi:",key)
    ForwardMessageToAirPi(BuildMessageGET(key))
    value=getValueForKey(settingsDatabase,key)
    if(value==None):
        value="INVALID_SETTING"
    global messagesForClient
    messagesForClient.put(BuildMessageGET_OK("G",key,value))
    

#process messages coming from the settings app (external devices)
def processMessageFromClient(msg):
    print("Message from client",msg)
    global messagesForClient
    cmd,data=ParseMessage(msg)
    if(cmd=="HELLO_OK"):
        global lastHELLO_OKmessage
        lastHELLO_OKmessage=time.time()
    elif(cmd=="CHANGE"):
        key,value=ParseMessageData(data)
        processChangeMessage(key,value)
    elif(cmd=="GET"):
        processGetMessage(data)
    else:
        print("Unknown command")


#Listen for messages coming from the air pi
#add them to the message queue such that they can be forwarded to External devices
#blocking, execute in its own threaad
def ListenForAirPiMessages():
    while(True):
        receiveSock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
        receiveSock.bind(('localhost',9292))
        data=receiveSock.recv(1024)
        #if(data):
        global messagesForClient
        messagesForClient.put(data.decode())


#create a new Thread which listens for incoming responses from the air pi and adds them to the queue,
#such that they can be forwarded to the app via TCP connection
#TODO maybe start only as soon as a client connected to save cpu
thread1 = Thread(target = ListenForAirPiMessages)
thread1.start()

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# Bind the socket to localhost
server_address = ("0.0.0.0", 5601)
print('starting up on %s port %s' % server_address)
sock.bind(server_address)
sock.listen(1)


while True:
    print('waiting for a connection')
    #this one hangs until someone connects
    #(exactly what we want,since blocking operations use almost no CPU)
    sock.settimeout(None)
    connection,client_address = sock.accept()
    try:
        print('client connected:', client_address)
        #But as soon as there is an connection, we use a timeout - else we cannot implement the
        #Hello->HelloOK connection check that is required to test if the connection was forcibly closed
        #without notifying the server (e.g. the user disabled WIFI but didn't close the app first)
        connection.settimeout(0.5)
        lastHELLOmessage=time.time()
        lastHELLO_OKmessage=time.time()
        lineBuffer = ''
        while True:
            #first,try to receive some data from the socket, and parse it into a line
            try:
                data = connection.recv(1024*1024).decode()
                #Parse bytes into lines (makeFile() caused issues I could not solve)
                for x in data:
                    if(x=='\n'):
                        #print('Received line',lineBuffer)
                        processMessageFromClient(lineBuffer)
                        lineBuffer=''
                    else:
                        lineBuffer+=x
            except socket.timeout as e:
                #print("Timeout exception",e)
                #Receiving timeout here is no problem
                pass
            #second,check if there are any messages in the queue that handles communication with the
            #air pi communication thread
            try:
                while (True):
                    message=messagesForClient.get_nowait()
                    #print("Message in queue"+message)
                    connection.sendall((message+"\n").encode())
                    print("Sent to external device:",message)
            except Exception as e:
                #print("Queue exception",e)
                #when the queue is empty it throws an exception,do nothing in this case
                pass
            #third, check first if we didn't receive a HELLO_OK message in the last X=5 seconds. If so, we can assume
            #that the connection was closed but we didn't get notified
            currTime=time.time()
            if((currTime-lastHELLO_OKmessage)>5.0):
                print("No response from client in >5 seconds. CLosing connecction")
                print(currTime-lastHELLO_OKmessage)
                connection.close()
            #if there was no error yet, send the 'HELLO' message to the client (every 1 seconds)
            if((currTime-lastHELLOmessage)>=1.0):
                lastHELLOmessage=currTime
                messagesForClient.put(BuildMessageHELLO())
    except Exception as e:
        print("ExceptionC",e)
        #don't forget to reset the the message queue
        try:
            while True:
                messagesForClient.get_nowait()
        except Exception as e:
            pass