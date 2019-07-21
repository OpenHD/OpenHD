# Change to script folder and execute main entry point
TTY=`tty`


case $TTY in
    /dev/tty1) # TX/RX
	echo "tty1"
	
	python /root/wifibroadcast_misc/gpio-IsAir.py
	
	i2cdetect -y 1 | grep  "70: 70"
        grepRet=$?
        if [[ $grepRet -eq 0 ]] ; then
            /usr/bin/python3.5 /home/pi/cameracontrol/InitArduCamV21Ch1.py
        fi
	
	CAM=`/usr/bin/vcgencmd get_camera | nice grep -c detected=1`
	
        i2cdetect -y 0 | grep  "30: -- -- -- -- -- -- -- -- -- -- -- 3b -- -- -- --"
        grepRet=$?
        if [[ $grepRet -eq 0 ]] ; then
			/usr/bin/python3.5 /home/pi/RemoteSettings/Air/RemoteSettingSyncAir.py
			CAM="1"
			echo "0" > /tmp/ReadyToGo
        else
			if [ -e /tmp/Air ]; then
				echo "force boot as Air via GPIO"
				CAM="1"
			fi
			
			if [ "$CAM" == "0" ]; then # if we are RX ...
				/home/pi/RemoteSettings/Ground/helper/AirRSSI.sh &
				/home/pi/RemoteSettings/Ground/helper/DisplayProgram/DisplayProgram &
				/home/pi/RemoteSettings/Ground/helper/ConfigureNics.sh
				retCode=$?
				if [ $retCode == 1 ]; then
					# Ret Code is 1. joystick selected as control
					/usr/bin/python3.5 /home/pi/RemoteSettings/Ground/RemoteSettingsSync.py -ControlVia joystick
				fi

				if [ $retCode == 2 ]; then
					# Ret code is 2  GPIO  selected as control
					/usr/bin/python3.5 /home/pi/RemoteSettings/Ground/RemoteSettingsSync.py -ControlVia GPIO
				fi
	
				echo "0" > /tmp/ReadyToGo
			else # else we are TX ...
				/usr/bin/python3.5 /home/pi/RemoteSettings/Air/RemoteSettingSyncAir.py
				echo "0" > /tmp/ReadyToGo
			fi
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
