function ground_remote_settings2(){
	echo "starting ground_remote_settings2\n"
	/home/pi/RemoteSettings2/src_scripts/G_TxRemoteSettings2.sh &
	sleep 0.5
	/home/pi/RemoteSettings2/src_scripts/G_RxRemoteSettings2.sh &
	sleep 0.5
	while true
	do 
		echo "starting ServerGround.py"
		python3 /home/pi/RemoteSettings2/src_python/ServerGround.py
		echo "restarting ServerGround.py"
		sleep 1
	done
}

function air_remote_settings2(){
	echo "starting air_remote_settings2\n"
	/home/pi/RemoteSettings2/src_scripts/A_TxRemoteSettings2.sh &
	sleep 0.5
	/home/pi/RemoteSettings2/src_scripts/A_RxRemoteSettings2.sh &
	sleep 0.5
	while true
	do
		echo "starting ServerAir.py"
                python3 /home/pi/RemoteSettings2/src_python/ServerAir.py
                echo "restarting ServerAir.py"
                sleep 1
	done
}

