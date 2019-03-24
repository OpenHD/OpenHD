
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
def ForwardMessageToAirPi(Message):
    #Send data to the air pi on the right UDP socket
    print("forwarding message to air",Message)
    sendSock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    sendBytes=sendSock.sendto(Message.encode(),('localhost', 9090))



#Currently unused, but this would remove the need for the message queue on the ground pi
#forward message to air pi via UDP and wait up to X=2 seconds for a response.
#return response on succes, Null otherwise
def ForwardMessageToAirPiAndAwaitResponse(Message):
    #first, bind the port we are going to receive the response from (if succesfull)
    receiveSock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    receiveSock.bind(('localhost',9090))
    receiveSock.settimeout(2.0)
    #then, send data to the air pi on another UDP socket
    sendSock=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    sendSock.sendto(Message.encode(),('localhost', 9090))
    try:
        data=receiveSock.recv(1024)
        print("Received from air pi:",data.decode())
        return data.decode()
    except socket.timeout as e:
        print("Nothing received from air pi in time",e)
    return None
