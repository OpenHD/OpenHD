#!/usr/bin/env python3

import socket
from threading import Thread
from time import sleep
import sys

remote_addresses = []

def data_thread(data_in_port, data_out_port):
    data_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    data_socket.bind(("", data_in_port))
    print("Forwarding from port " + str(data_in_port) + " to " + str(data_out_port))
    while True:
        try:
            data, addr = data_socket.recvfrom(1500)
            for address in remote_addresses:
                data_socket.sendto(data, (address, data_out_port))
        except Exception as e:
            print("Error in data stream: " + str(e))



def command_thread(command_port):
    command_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    command_socket.bind(("localhost", command_port))
    print("Command port: " + str(command_port))
    while True:
        try:
            data, addr = command_socket.recvfrom(1024)
            received_command = data.decode("utf-8").rstrip().split(' ')
            if len(received_command) == 2:
                command = received_command[0]
                arg = received_command[1]
                if command == "add":
                    print("Adding remote address: " + arg)
                    if not arg in remote_addresses:
                        remote_addresses.append(arg)
                elif command == "del":
                    print("Removing remote address: " + arg)
                    remote_addresses.remove(arg)

        except Exception as e:
            print("Error in command API: " + str(e))
        sleep(0.1)


if len(sys.argv) < 4:
    print("Usage:")
    print("")
    print("./udpplitter.py PORT_COMMAND PORT_IN PORT_OUT")
    print("")
    print("UDP API:")
    print("")
    print("The UDP command port listens for 2 commands:")
    print("add 192.168.2.20")
    print("del 192.168.2.20")
    print("")
    print("When add is called, the address will be added to the list")
    print("of remote addresses that data will be forwarded to.")
    exit(1)

command_port  = int(sys.argv[1])
data_in_port  = int(sys.argv[2])
data_out_port = int(sys.argv[3])

data_thread_handle = Thread(target=data_thread, args=(data_in_port, data_out_port))
command_thread_handle = Thread(target=command_thread, args=(command_port,))

data_thread_handle.start()
command_thread_handle.start()

data_thread_handle.join()
command_thread_handle.join()

