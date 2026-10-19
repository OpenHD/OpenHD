import socket,select,struct,sys,time
sockets=[]
try:
    for port in (5600,5601):
        s=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
        s.setsockopt(socket.SOL_SOCKET,socket.SO_RCVBUF,4*1024*1024)
        s.bind(('127.0.0.1',port)); sockets.append(s)
    tcp=socket.create_connection(('127.0.0.1',5760),timeout=5)
    tcp.settimeout(None); sockets.append(tcp)
    out=sys.stdout.buffer
    last=time.monotonic()
    while True:
        ready,_,_=select.select(sockets,[],[],5)
        for s in ready:
            data=s.recv(65535)
            if s is tcp:
                if not data: raise EOFError('OpenHD telemetry socket closed')
                continue
            port=s.getsockname()[1]
            out.write(struct.pack('!HH',port,len(data))+data); out.flush()
            last=time.monotonic()
        if time.monotonic()-last>15: raise TimeoutError('No video for 15 seconds')
finally:
    for s in sockets: s.close()