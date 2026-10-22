"""Measure local RTP delivery by wall time and save H264 for decoder validation."""
import argparse
import json
import socket
import struct
import time
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument('output',type=Path)
p.add_argument('--seconds',type=float,default=12)
p.add_argument('--port',type=int,default=5601)
a=p.parse_args()
a.output.parent.mkdir(parents=True,exist_ok=True)
s=socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
s.setsockopt(socket.SOL_SOCKET,socket.SO_RCVBUF,4*1024*1024)
s.bind(('127.0.0.1',a.port))
s.settimeout(1)
deadline=time.monotonic()+40
started=None
last_packet=None
max_frame_gap=0
frames=[]
packets=invalid=sequence_gaps=duplicates=0
previous_seq=None
last_timestamp=None
previous_ssrc=None
stream_changes=timestamp_discontinuities=0
nal_types={}
with a.output.with_suffix('.h264').open('wb') as out:
    while time.monotonic()<deadline+(2 if started is not None else 0):
        try:data,_=s.recvfrom(65535)
        except socket.timeout:continue
        now=time.monotonic()
        if len(data)<12 or data[0]>>6!=2:
            invalid+=1
            continue
        if started is None:
            started=now
            deadline=now+a.seconds
        packets+=1
        last_packet=now
        seq,ts,ssrc=struct.unpack_from('!HII',data,2)
        if previous_ssrc is not None and previous_ssrc!=ssrc:
            stream_changes+=1
            previous_seq=last_timestamp=None
        previous_ssrc=ssrc
        if previous_seq is not None:
            delta=(seq-previous_seq)&65535
            if delta==0:duplicates+=1
            elif delta>1 and delta<32768:sequence_gaps+=delta-1
        previous_seq=seq
        offset=12+4*(data[0]&15)
        if data[0]&16:
            if len(data)<offset+4:
                invalid+=1
                continue
            offset+=4+4*struct.unpack_from('!H',data,offset+2)[0]
        payload=data[offset:len(data)-(data[-1] if data[0]&32 else 0)]
        if not payload:
            invalid+=1
            continue
        kind=payload[0]&31
        nal_types[kind]=nal_types.get(kind,0)+1
        if 1<=kind<=23:
            out.write(b'\x00\x00\x00\x01'+payload)
        elif kind==24:
            pos=1
            while pos+2<=len(payload):
                length=struct.unpack_from('!H',payload,pos)[0]
                pos+=2
                if pos+length>len(payload):
                    invalid+=1
                    break
                out.write(b'\x00\x00\x00\x01'+payload[pos:pos+length])
                pos+=length
        elif kind==28 and len(payload)>=2:
            if payload[1]&128:
                out.write(b'\x00\x00\x00\x01'+bytes([(payload[0]&224)|(payload[1]&31)]))
            out.write(payload[2:])
        if data[1]&128 and ts!=last_timestamp:
            if last_timestamp is not None and ((ts-last_timestamp)&0xffffffff)>90000:
                timestamp_discontinuities+=1
            if frames:
                max_frame_gap=max(max_frame_gap,now-frames[-1]['wall'])
            frames.append({'wall':now,'timestamp':ts})
            last_timestamp=ts
            # End on a complete access unit so the final saved frame decodes.
            if now>=deadline:break
s.close()
finished=time.monotonic()
elapsed=frames[-1]['wall']-frames[0]['wall'] if len(frames)>1 else 0
observed=finished-started if started is not None else 0
trailing_idle=finished-frames[-1]['wall'] if frames else observed
full_duration=bool(frames and elapsed>=a.seconds-0.1 and trailing_idle<1 and max_frame_gap<1
                   and not stream_changes and not timestamp_discontinuities)
rtp_seconds=((frames[-1]['timestamp']-frames[0]['timestamp'])&0xffffffff)/90000 if len(frames)>1 else 0
report={'packets':packets,'invalid_packets':invalid,'sequence_gaps':sequence_gaps,
        'requested_seconds':a.seconds,'observation_seconds':observed,
        'trailing_frame_idle_seconds':trailing_idle,'max_frame_gap_seconds':max_frame_gap,
        'continuous_for_requested_duration':full_duration,
        'ssrc_changes':stream_changes,'timestamp_discontinuities':timestamp_discontinuities,
        'lossless_rtp':bool(packets) and not (invalid or sequence_gaps or duplicates or stream_changes or timestamp_discontinuities),
        'observation_fps':len(frames)/observed if observed else None,
        'duplicates':duplicates,'distinct_frame_timestamps':len(frames),
        'wall_interval_seconds':elapsed,'wall_fps':(len(frames)-1)/elapsed if elapsed else None,
        'rtp_interval_seconds':rtp_seconds,'rtp_timestamp_fps':(len(frames)-1)/rtp_seconds if rtp_seconds else None,
        'nal_types':nal_types}
a.output.with_suffix('.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
if len(frames)<2 or not full_duration:raise SystemExit(1)
