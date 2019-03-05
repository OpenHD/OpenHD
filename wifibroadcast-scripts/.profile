# Change to script folder and execute main entry point
TTY=`tty`


case $TTY in
    /dev/tty1) # TX/RX
	echo "tty1"
	CAM=`/usr/bin/vcgencmd get_camera | nice grep -c detected=1`
	if [ "$CAM" == "0" ]; then # if we are RX ...
		/home/pi/RemoteSettings/Ground/helper/ConfigureNics.sh
		/usr/bin/python3.5 /home/pi/RemoteSettings/Ground/RemoteSettingsSync.py
		echo "0" > /tmp/ReadyToGo
	else # else we are TX ...
		/usr/bin/python3.5 /home/pi/RemoteSettings/Air/RemoteSettingSyncAir.py
		echo "0" > /tmp/ReadyToGo
	fi


;;

esac


while [ ! -f /tmp/ReadyToGo ]
do
  echo "sleep..."
  sleep 1
done
echo "go..."

cd /home/pi/wifibroadcast-scripts
source main.sh
