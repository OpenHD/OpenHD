import socket


from time import sleep
import argparse
import threading
lock = threading.Lock()

InMsgBand5 = bytearray(b'5') #Air to 5
InMsgBand10 = bytearray(b'a') #Air to 10
InMsgBand20 = bytearray(b'0') #Air to 20
InMsgCameraTypeRPi = bytearray(b'RPi')
InMsgCameraTypeRPiAndSecondary = bytearray(b'RPiAndSecondary')
InMsgCameraTypeSecondary = bytearray(b'Secondary')



def SendDataToBandSwitcher(MessageBuf):
    UDP_PORT = 4322
    sockToAir = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
    sockToAir.sendto(MessageBuf, ('127.0.0.1', UDP_PORT))

def SendDataToCameraControl(MessageBuf):
    UDP_PORT = 4323
    sockToAir = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
    sockToAir.sendto( MessageBuf, ('127.0.0.1', UDP_PORT))


UDP_IP = ""
UDP_PORT = 4321 #3033 - UDP DownLink from Ground
sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))
while True:
    data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes

    if data == InMsgBand5:
        print("InMsgBand5\n")
        SendDataToBandSwitcher(InMsgBand5)

    if data == InMsgBand10:
        print("InMsgBand10\n")
        SendDataToBandSwitcher(InMsgBand10)
                
    if data == InMsgBand20:
        print("InMsgBand20\n")
        SendDataToBandSwitcher(InMsgBand20)


    if data == InMsgCameraTypeRPi:
            SendDataToCameraControl(InMsgCameraTypeRPi)

    if data == InMsgCameraTypeRPiAndSecondary:
            SendDataToCameraControl(InMsgCameraTypeRPiAndSecondary)

    if data == InMsgCameraTypeSecondary:
        SendDataToCameraControl(InMsgCameraTypeSecondary)
