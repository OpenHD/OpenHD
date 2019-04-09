
#Forward commands like changing settings to the air pi via ez-wb
#since this is a lossy connection, we have to use our own synchronisation mechanism

#Forwarding data to the air pi works <? HOW EXATCLY ?>

#Idea:


#when the server running on the ground pi received a 'change settings' command (e.g from the app) we 
#add a checksum to the message and send it out via udp to the air pi.

#The air pi listens for incoming messages and this udp channel.

#Only when the checksum is right the air pi responds with a acknowledge message, else it
#doesn't send anything.


#The ground server waits for the response from the air pi, and if there is no reponse in X=5 seconds 
#It does not change any settings


import socket
import sys
from io import StringIO
import io
import threading
from threading import Thread


#Asynchronous (does not block, assuming the UDP socket accepts data)
#make sure to start listening on the response UDP socket before sending
#call this on ground pi only, data is sent via wfb
def SendMessageToAirPi(message):
    #Send data to the air pi on the right UDP socket
    sendSock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    sendSock.sendto(message.encode(),('localhost', 5701))
    print("sent to air pi:",message)


#call this on the air pi only, data is sent via wfb
def SendMessageToGroundPi(message):
    sendSock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    sendSock.sendto((message).encode(),('localhost', 5702))
    print("Sent to ground pi:",message)


