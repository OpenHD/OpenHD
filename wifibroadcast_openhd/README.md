## Summary
This code originated from https://github.com/svpcom/wifibroadcast  
It was re-written in c++ with the intention to reduce latency, improve syntax, improve documentation 
and modularize the FEC Enc/Dec and Encryption/Decryption part. (represented by FECEncoder/Decoder and Encryptor/Decryptor now).
By doing so I was able to reduce latency quite a lot (even though the fix was one line of code in the end) and
write simple unit tests that don't require a wifi card.
I also added some new features,listed:
#### 1) variable block size:
If you are transmitting h264,h265 or mjpeg video, automatically end a block with the end of each frame. This reduces latency and can increase resiliency against packet loss
#### 2) disable FEC:
For telemetry or RC data, FEC doesn't really make sense. By disabling FEC you get the same properties as "true UDP".
#### 3) simplified settings:
The tx and rx pipeline only have to match on the radio_port, the rest is done automatically

## Notable parameter changes:
-p is now the percentage of FEC packets (previosuly radio_port) and -r is now the radio_port   
This means, for a fixed block length, you don't have to supply k:n anymore, but rather -k (number of data packets per block) and -p (fec percentage).
The tx is going to print this input in "old" k:n terms for you.
   
## Example pipelines
### 1) Transmitting rtp encapsulated h264/h265/mjpeg video:
**./wfb_tx -k h264 -p 50**\
This reads as follow: variable block length for rtp h264 video,with an overhead of
FEC packets of 50% (if your input stream is 20MBit/s, the used bandwidth is going to be ~30MBit/s)
### 2) Fixed block length (for whatever reason):
**./wfb_tx -k 8 -p 50**\
This reads as follow: fixed block length where each block contains 8 data packets and 8*50/100= 4 fec packets.   
In "old k:n terms" this would be 8:12   
### 3) FEC disabled (udp-like) for telemetry (use only if your upper level deals with packet re-ordering, like mavlink):
**./wfb_tx -k 0**
   

## Information about using -k 0 or -k 1:
1) If tx uses -k 0 the rx forwards packets without duplicates but with possible packet re-ordering due to multiple wifi cards. Aka "just as you'd expect from UDP" but no duplicates to save bandwidth as soon as possible (if the upper level does re-sending of the same packets, for the lower wb level this is just a new packet, to not confuse anybody here, that'l still work).
2) If tx uses -k 1 and -p 100 (for exampe) this degenerates to "sending each packet twice" and the output on rx is going to be in-order, zero latency, but more than one rx card won't have a benificial effect
3) With -k>1 and -p anything the rx behaves just as you are used from svpcom-wifibroadcast. Just be aware that the -p value is an integer,
and "weird" -k -p combinations are rounded to the nearest integer (look at tx output in terminal)
   
## Information about encryption:
The encryption part serves 2 purposes: On the one hand,it encrypt the packets. On the other hand, it also "validates" packets. If the user-generated keys on the rx and tx do not match, the rx won't forward any packets. This can be used to basically "bind" one air pi to one ground pi.

## Overhead
If the link is not active (e.g. no data is feed into the tx) this layer does not send any packets (not even the session key packets). The used wifi bitrate is 0 in this case.
If the link is active (e.g. data is fed into the tx) the packet overhead is one packet every SESSION_KEY_ANNOUNCE_DELTA ms, 1+8+2=11 bytes per data packet, and -p % more packets.
If you don't need to be exact,just assume that the overhead is -p percent.

## Where to set FEC_K or FEC_PERCENTAGE
Only needs to be set on tx, rx configures itself automatically

### Building wfb_tx and wfb_rx
install libsodium-dev\
install libpcap-dev\
Then run make

### Building / using the test program
go into latencyTesting/SimpleTestProgram\
Then run make\
You can now start the tx on one card, the rx on another card and use the test program to measure throughput,latency,packet loss and more
