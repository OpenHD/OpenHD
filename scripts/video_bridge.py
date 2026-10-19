import argparse,pathlib,socket,struct,subprocess,time
parser=argparse.ArgumentParser(description='Forward both OpenHD RTP streams over SSH, reconnecting after interruptions.')
parser.add_argument('target',help='SSH destination, for example root@192.168.1.75')
parser.add_argument('--log-dir',type=pathlib.Path,default=pathlib.Path('out/video-bridge'))
args=parser.parse_args()
base=args.log_dir
base.mkdir(parents=True,exist_ok=True)
helper=pathlib.Path(__file__).with_name('video_bridge_remote.py')
subprocess.run(['scp','-o','BatchMode=yes','-o','ConnectTimeout=5',str(helper),args.target+':/tmp/openhd-video-bridge.py'],check=True)
stop=base/'stop-video-bridge'
cmd=['ssh','-T','-o','BatchMode=yes','-o','ConnectTimeout=5','-o','ServerAliveInterval=5','-o','ServerAliveCountMax=2',args.target,'python3 -u /tmp/openhd-video-bridge.py']
s=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
counts={5600:0,5601:0}
err=open(base/'video-bridge-ssh.log','ab',buffering=0)
while not stop.exists():
    p=None
    try:
        p=subprocess.Popen(cmd,stdout=subprocess.PIPE,stderr=err)
        def read(n):
            data=bytearray()
            while len(data)<n:
                part=p.stdout.read(n-len(data))
                if not part: raise EOFError('SSH video stream closed')
                data.extend(part)
            return data
        print('Connecting video bridge',flush=True)
        last=time.monotonic()
        while not stop.exists():
            port,n=struct.unpack('!HH',read(4))
            if port not in counts or not 12<=n<=65507: raise ValueError('Invalid video record')
            data=read(n); s.sendto(data,('127.0.0.1',port)); counts[port]+=1
            if time.monotonic()-last>=5:
                print(str(counts),flush=True);last=time.monotonic()
    except Exception as e:
        print('Reconnecting: '+str(e),flush=True)
    finally:
        if p:
            p.terminate()
            try:p.wait(timeout=5)
            except subprocess.TimeoutExpired:p.kill();p.wait()
    if not stop.exists():time.sleep(2)
s.close()