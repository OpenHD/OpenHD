
#!/bin/bash

/home/pi/RemoteSettings/Ground/TxForwardBandSwitcher.sh &
sleep 0.5
/home/pi/RemoteSettings/Ground/RxForwardToBandSwitcher.sh &
sleep 0.5
/home/pi/RemoteSettings/Ground/BandSwitcher.sh &
