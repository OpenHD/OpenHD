import socket
import time

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind(("0.0.0.0", 5600))
s.settimeout(10)
packets = []
deadline = time.time() + 10
while len(packets) < 300 and time.time() < deadline:
    try:
        data, _ = s.recvfrom(65535)
    except socket.timeout:
        break
    packets.append(data)

valid = []
for data in packets:
    if len(data) < 12:
        continue
    version = data[0] >> 6
    cc = data[0] & 0x0F
    header = 12 + cc * 4
    if data[0] & 0x10 and len(data) >= header + 4:
        ext_words = int.from_bytes(data[header + 2:header + 4], "big")
        header += 4 + ext_words * 4
    if version == 2 and header < len(data):
        valid.append((int.from_bytes(data[2:4], "big"), data[1] & 0x7F,
                      data[header] & 0x1F, len(data), data[:20].hex()))

print(f"packets={len(packets)} valid_rtp={len(valid)} invalid={len(packets)-len(valid)}")
for item in valid[:30]:
    print("seq=%d pt=%d nal=%d len=%d head=%s" % item)
if valid:
    gaps = sum(((b[0] - a[0]) & 0xFFFF) != 1 for a, b in zip(valid, valid[1:]))
    print(f"sequence_gaps={gaps}/{max(0, len(valid)-1)}")
