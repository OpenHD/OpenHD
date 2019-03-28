#!/bin/bash

DATARATE=$1
FREQ=$2
TXMODE=$3


function tmessage {
    if [ "$QUIET" == "N" ]; then
	echo $1 "$2"
    fi
}

function datarate_to_wifi_settings {
	case $DATARATE in
		1)
		UPLINK_WIFI_BITRATE=11
		TELEMETRY_WIFI_BITRATE=11
		VIDEO_WIFI_BITRATE=5.5
		;;
		2)
		UPLINK_WIFI_BITRATE=11
		TELEMETRY_WIFI_BITRATE=11
		VIDEO_WIFI_BITRATE=11
		;;
		3)
		UPLINK_WIFI_BITRATE=11
		TELEMETRY_WIFI_BITRATE=12
		VIDEO_WIFI_BITRATE=12
		;;
		4)
		UPLINK_WIFI_BITRATE=11
		TELEMETRY_WIFI_BITRATE=19.5
		VIDEO_WIFI_BITRATE=19.5
		;;
		5)
		UPLINK_WIFI_BITRATE=11
		TELEMETRY_WIFI_BITRATE=24
		VIDEO_WIFI_BITRATE=24
		;;
		6)
		UPLINK_WIFI_BITRATE=12
		TELEMETRY_WIFI_BITRATE=36
		VIDEO_WIFI_BITRATE=36
		;;
	esac
}



function collect_errorlog {
    sleep 3
    echo
	
    if nice dmesg | nice grep -q over-current; then
        echo "ERROR: Over-current detected - potential power supply problems!"
    fi

    # check for USB disconnects (due to power-supply problems)
    if nice dmesg | nice grep -q disconnect; then
        echo "ERROR: USB disconnect detected - potential power supply problems!"
    fi

    nice mount -o remount,rw /boot

    # check if over-temp or under-voltage occured
    if vcgencmd get_throttled | nice grep -q -v "0x0"; then
		TEMP=`cat /sys/class/thermal/thermal_zone0/temp`
		TEMP_C=$(($TEMP/1000))
		if [ "$TEMP_C" -lt 75 ]; then # it must be under-voltage
			echo
			echo "  ---------------------------------------------------------------------------------------------------"
			echo "  | ERROR: Under-Voltage detected on the TX Pi. Your Pi is not supplied with stable 5 Volts.        |"
			echo "  | Either your power-supply or wiring is not sufficent, check the wiring instructions in the Wiki. |"
			echo "  | Video Bitrate will be reduced to 1000kbit to reduce current consumption!                        |"
			echo "  ---------------------------------------------------------------------------------------------------"
			echo
			echo "  ---------------------------------------------------------------------------------------------------" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
			echo "  | ERROR: Under-Voltage detected on the TX Pi. Your Pi is not supplied with stable 5 Volts.        |" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
			echo "  | Either your power-supply or wiring is not sufficent, check the wiring instructions in the Wiki. |" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
			echo "  | Video Bitrate will be reduced to 1000kbit to reduce current consumption!                        |" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
			echo "  | When you have fixed wiring/power-supply, delete this file and make sure it doesn't re-appear!   |" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
			echo "  ---------------------------------------------------------------------------------------------------" >> /boot/UNDERVOLTAGE-ERROR!!!.txt
		fi
    fi

    mv /boot/errorlog.txt /boot/errorlog-old.txt > /dev/null 2>&1
    mv /boot/errorlog.png /boot/errorlog-old.png > /dev/null 2>&1
    echo -n "Camera: "
    nice /usr/bin/vcgencmd get_camera
    uptime >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt
    echo -n "Camera: " >>/boot/errorlog.txt
    nice /usr/bin/vcgencmd get_camera >>/boot/errorlog.txt
    echo
    nice dmesg | nice grep disconnect
    nice dmesg | nice grep over-current
    nice dmesg | nice grep disconnect >>/boot/errorlog.txt
    nice dmesg | nice grep over-current >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt
    echo

    NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb`

    for NIC in $NICS
    do
		iwconfig $NIC | grep $NIC
    done
	
    echo
    echo "Detected USB devices:"
    lsusb

    nice iwconfig >>/boot/errorlog.txt > /dev/null 2>&1
    echo >>/boot/errorlog.txt
    nice ifconfig >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt

    nice iw reg get >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt

    nice iw list >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt


    nice ps ax >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt

    nice df -h >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt

    nice mount >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt

    nice fdisk -l /dev/mmcblk0 >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt

    nice lsmod >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt

    nice lsusb >>/boot/errorlog.txt
    nice lsusb -v >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt
    nice ls -la /dev >>/boot/errorlog.txt
    nice ls -la /dev/input >>/boot/errorlog.txt
    echo
    nice vcgencmd measure_temp
    nice vcgencmd get_throttled
    echo >>/boot/errorlog.txt
    nice vcgencmd measure_volts >>/boot/errorlog.txt
    nice vcgencmd measure_temp >>/boot/errorlog.txt
    nice vcgencmd get_throttled >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt
    nice vcgencmd get_config int >>/boot/errorlog.txt

    nice /home/pi/wifibroadcast-misc/raspi2png -p /boot/errorlog.png
    echo >>/boot/errorlog.txt
    nice dmesg >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt

    nice cat /etc/modprobe.d/rt2800usb.conf >> /boot/errorlog.txt
    nice cat /etc/modprobe.d/ath9k_htc.conf >> /boot/errorlog.txt
    nice cat /etc/modprobe.d/ath9k_hw.conf >> /boot/errorlog.txt

    echo >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt
    nice cat /boot/openhd-settings-1.txt | egrep -v "^(#|$)" >> /boot/errorlog.txt
    echo >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt
    nice cat /boot/osdconfig.txt | egrep -v "^(//|$)" >> /boot/errorlog.txt
    echo >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt
    nice cat /boot/joyconfig.txt | egrep -v "^(//|$)" >> /boot/errorlog.txt
    echo >>/boot/errorlog.txt
    echo >>/boot/errorlog.txt
    nice cat /boot/apconfig.txt | egrep -v "^(#|$)" >> /boot/errorlog.txt

    sync
    nice mount -o remount,ro /boot
}



function detect_nics {
	tmessage "Setting up wifi cards ... "
	echo

	# set reg domain to DE to allow channel 12 and 13 for hotspot
	iw reg set DE

	NUM_CARDS=-1
	NICSWL=`ls /sys/class/net | nice grep wlan`

	for NIC in $NICSWL
	do
	    # set MTU to 2304
	    ifconfig $NIC mtu 2304
	    # re-name wifi interface to MAC address
	    NAME=`cat /sys/class/net/$NIC/address`
	    ip link set $NIC name ${NAME//:}
	    let "NUM_CARDS++"
	    #sleep 0.1
	done

	if [ "$NUM_CARDS" == "-1" ]; then
	    echo "ERROR: No wifi cards detected"
	    collect_errorlog
	    sleep 365d
	fi


    NICS=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v relay | nice grep -v wifihotspot`
#	echo "NICS: $NICS"

	if [ "$TXMODE" != "single" ]; then
	    for i in $(eval echo {0..$NUM_CARDS})
	    do
		prepare_nic ${MAC_TX[$i]} ${FREQ_TX[$i]}
		sleep 0.1
	    done
	else
		for NIC in $NICS
		do
			prepare_nic $NIC $FREQ "$TXPOWER"
			sleep 0.1
		done
	fi

	echo $NICS > /tmp/NICS_LIST
	touch /tmp/nics_configured # let other processes know nics are setup and ready
}




function prepare_nic {
    DRIVER=`cat /sys/class/net/$1/device/uevent | nice grep DRIVER | sed 's/DRIVER=//'`
    if [ "$DRIVER" != "rt2800usb" ] && [ "$DRIVER" != "mt7601u" ] && [ "$DRIVER" != "ath9k_htc" ]; then
	tmessage "WARNING: Unsupported or experimental wifi card: $DRIVER"
    fi

    case $DRIVER in
    *881[24]au)
        DRIVER=rtl88xxau
        ;;
    esac

    tmessage -n "Setting up $1: "
    if [ "$DRIVER" == "ath9k_htc" ]; then # set bitrates for Atheros via iw
	ifconfig $1 up || {
	    echo
	    echo "ERROR: Bringing up interface $1 failed!"
	    collect_errorlog
	    sleep 365d
	}
	sleep 0.2

	    tmessage -n "bitrate "
	    if [ "$VIDEO_WIFI_BITRATE" != "19.5" ]; then # only set bitrate if something else than 19.5 is requested (19.5 is default compiled in ath9k_htc firmware)
		tmessage -n "$VIDEO_WIFI_BITRATE Mbit "
		iw dev $1 set bitrates legacy-2.4 $VIDEO_WIFI_BITRATE || {
		    echo
		    echo "ERROR: Setting bitrate on $1 failed!"
		    collect_errorlog
		    sleep 365d
		}
	    else
		tmessage -n "$VIDEO_WIFI_BITRATE Mbit "
	    fi
	    sleep 0.2
	    tmessage -n "done. "

	ifconfig $1 down || {
	    echo
	    echo "ERROR: Bringing down interface $1 failed!"
	    collect_errorlog
	    sleep 365d
	}
	sleep 0.2

	tmessage -n "monitor mode.. "
	iw dev $1 set monitor none || {
	    echo
	    echo "ERROR: Setting monitor mode on $1 failed!"
	    collect_errorlog
	    sleep 365d
	}
	sleep 0.2
	tmessage -n "done. "

	ifconfig $1 up || {
	    echo
	    echo "ERROR: Bringing up interface $1 failed!"
	    collect_errorlog
	    sleep 365d
	}
	sleep 0.2

	if [ "$2" != "0" ]; then
	    tmessage -n "frequency $2 MHz.. "
	    iw dev $1 set freq $2 || {
		echo
		echo "ERROR: Setting frequency $2 MHz on $1 failed!"
		collect_errorlog
		sleep 365d
	    }
	    tmessage "done!"
	else
	    echo
	fi

    fi

    if [ "$DRIVER" == "rt2800usb" ] || [ "$DRIVER" == "mt7601u" ] || [ "$DRIVER" == "rtl8192cu" ] || [ "$DRIVER" == "rtl88xxau" ]; then # do not set bitrate for Ralink, Mediatek, Realtek, done through tx parameter
	tmessage -n "monitor mode.. "
	iw dev $1 set monitor none || {
	    echo
	    echo "ERROR: Setting monitor mode on $1 failed!"
	    collect_errorlog
	    sleep 365d
	}
	sleep 0.2
	tmessage -n "done. "

	#tmessage -n "bringing up.. "
	ifconfig $1 up || {
	    echo
	    echo "ERROR: Bringing up interface $1 failed!"
	    collect_errorlog
	    sleep 365d
	}
	sleep 0.2
	#tmessage -n "done. "

	if [ "$2" != "0" ]; then
	    tmessage -n "frequency $2 MHz.. "
	    iw dev $1 set freq $2 || {
		echo
		echo "ERROR: Setting frequency $2 MHz on $1 failed!"
		collect_errorlog
		sleep 365d
	    }
	    tmessage "done!"
	else
	    echo
	fi

	if  [ "$DRIVER" == "rtl88xxau" -a -n "$3" ]; then
  	    tmessage -n "TX power $3.. "
	    iw dev $1 set txpower fixed $3 || {
	        echo
	        echo "ERROR: Setting TX power to $3 on $1 failed!"
	        collect_errorlog
	        sleep 365d
	    }
	    sleep 0.2
	    tmessage -n "done. "
	fi

    fi
}

datarate_to_wifi_settings

detect_nics
exit
