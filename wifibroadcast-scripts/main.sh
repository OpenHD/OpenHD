###############################################################################
# The main entrypoint for the OpendHD system.							
# Based heavily on the EZ-Wifibroadcast .profile script.
###############################################################################



###############################################################################
# Setup global variables
###############################################################################
source global_functions.sh

TTY=`tty`
#TTY="/dev/tty1"

# GPIO variable
CONFIGFILE=`/root/wifibroadcast_misc/gpio-config.py`

export PATH=/home/pi/wifibroadcast-status:${PATH}

autoenable_i2c_vc

# Check for the camera
check_camera_attached

# Read the config file
read_config_file

echo "-------------------------------------"
echo "SETTINGS FILE: $CONFIGFILE"
echo "-------------------------------------"


# Set the wifi parameters based on the selected datarate
datarate_to_wifi_settings

if [ "$CAM" == "0" ]; then
# For debugging viewing the different tty consoles can be useful. Set in settings
chvt $TTY_CONSOLE

# Set the approperiate display font size
set_font_for_resolution

# Set the specififc video player based on the fps
set_video_player_based_fps
fi	

# Fixed video values
VIDEO_UDP_BLOCKSIZE=1024
TELEMETRY_UDP_BLOCKSIZE=128

RELAY_VIDEO_BLOCKS=8
RELAY_VIDEO_FECS=4
RELAY_VIDEO_BLOCKLENGTH=1024

RSSI_UDP_PORT=5003

# Set telemetry settings based on telemetry type 
get_telemetry_settings

# cts protection
set_cts_protection


### FLIGHT CONTROLLER AND TELEMETRY SETTINGS
FC_TELEMETRY_STTY_OPTIONS="-icrnl -ocrnl -imaxbel -opost -isig -icanon -echo -echoe -ixoff -ixon"
EXTERNAL_TELEMETRY_SERIALPORT_GROUND_STTY_OPTIONS="-icrnl -ocrnl -imaxbel -opost -isig -icanon -echo -echoe -ixoff -ixon"
TELEMETRY_OUTPUT_SERIALPORT_GROUND_STTY_OPTIONS="-icrnl -ocrnl -imaxbel -opost -isig -icanon -echo -echoe -ixoff -ixon"


###############################################################################
# Include sub sources
###############################################################################
source tx_rx_functions.sh
source osd_tx_rx_functions.sh
source uplink_functions.sh

if [ "$CAM" == "0" ]; then
	source rc_tx_rx_functions.sh	
	source rssi_rx_functions.sh
	source screenshot_functions.sh
	source alive_functions.sh
	source video_save_functions.sh
	source tether_functions.sh
	source hotspot_functions.sh
fi	


###############################################################################
# Execute the different segments of the system on different TTY consoles
###############################################################################

# Setup consoles that run on BOTH AirPi and GroundPi
case $TTY in
    /dev/tty1) # TX/RX
		MAIN_TX_RX_FUNCTION
    ;;
    /dev/tty2) # OSD
		MAIN_OSD_TX_RX_FUNCTION
    ;;
    /dev/tty10) # uplink
		MAIN_UPLINK_FUNCTION
    ;;
    /dev/tty11) # tty for dhcp and login
	echo "================== eth0 DHCP client (tty11) ==========================="
	# sleep until everything else is loaded (atheros cards and usb flakyness ...)
	#sleep 6
	
	if [ "$CAM" != "0" ] && [ "$DEBUG" == "Y" ] || [ "$CAM" == "0" ]; then
	
	        if [ "$CAM" == "0" ]; then
	            OHDHOSTNAME="openhd-GroundPi"
	        else
	            OHDHOSTNAME="openhd-AirPi"
	        fi
		
	# only configure ethernet network interface via DHCP if ethernet hotspot is disabled
	if [ "$ETHERNET_HOTSPOT" == "N" ]; then
		# disabled loop, as usual, everything is flaky on the Pi, gives kernel stall messages ...
		nice ifconfig eth0 up
		sleep 5
		    if cat /sys/class/net/eth0/carrier | nice grep -q 1; then
			echo "Ethernet connection detected"
			CARRIER=1
			if nice pump -i eth0 --no-ntp -h $OHDHOSTNAME; then
			    ping -n -q -c 1 1.1.1.1
			else
			    ps -ef | nice grep "pump -i eth0" | nice grep -v grep | awk '{print $2}' | xargs kill -9
			    nice ifconfig eth0 down
			    echo "DHCP failed"
			fi
		    else
			echo "No ethernet connection detected"
		    fi
	else
	    echo "Ethernet Hotspot enabled, doing nothing"
	fi
	sleep 365d
      fi
    ;;
    /dev/tty12) # tty for local interactive login
	echo
	if [ "$CAM" == "0" ]; then
	    echo -n "Welcome to OpenHD"
	    read -p "Press <enter> to login"
	    killall osd
	    rw
	else
	    echo -n "Welcome to OpenHD"
	    read -p "Press <enter> to login"
	    rw
	fi
    ;;
esac

# Setup consoles that run ONLY on GroundPi

if [ "$CAM" == "0" ]; then
case $TTY in
    /dev/tty3) # RC Control
		MAIN_RC_TX_RX_FUNCTION
    ;;
    /dev/tty4) # unused
		MAIN_RSSI_RX_FUNCTION		
    ;;
    /dev/tty5) # screenshot stuff
		MAIN_SCREENSHOT_FUNCTION
    ;;
    /dev/tty6) # Save of video after flight
		MAIN_VIDEO_SAVE_FUNCTION	
    ;;
    /dev/tty7) # check tether	
		MAIN_TETHER_FUNCTION
    ;;
    /dev/tty8) # check hotspot
		MAIN_HOTSPOT_FUNCTION
    ;;
    /dev/tty9) # check alive
		MAIN_ALIVE_FUNCTION
    ;;
    *) # all other ttys used for interactive login
	if [ "$CAM" == "0" ]; then
	    echo "Welcome to OpenHD (GroundPi) - type 'ro' to switch filesystems back to read-only"
	    rw
	else
	    echo "Welcome to OpenHD (AirPi) - type 'ro' to switch filesystems back to read-only"
	    rw
	fi
    ;;

esac

fi
