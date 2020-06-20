
#!/bin/bash

/usr/local/share/RemoteSettings/Ground/TxForwardBandSwitcher.sh &
sleep 0.5
/usr/local/share/RemoteSettings/Ground/RxForwardToBandSwitcher.sh &
sleep 0.5
/usr/local/share/RemoteSettings/Ground/BandSwitcher.sh &
