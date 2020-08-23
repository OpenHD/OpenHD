###############################################################################
# The main entrypoint for the OpendHD system.							
# Based heavily on the EZ-Wifibroadcast .profile script.
###############################################################################



###############################################################################
# Setup global variables
###############################################################################
source global_functions.sh

TTY=`tty`


# GPIO variable
CONFIGFILE=`/usr/local/bin/gpio-config.py`

export PATH=/usr/local/bin:${PATH}

create_fifos

# don't allow anything to proceed until the fifos are created
while [ ! -f "/var/run/openhd/fifoready" ]; do
    sleep 0.5
done

autoenable_i2c_vc

check_camera_attached

check_hdmi_csi_attached

start_microservices

check_lifepowered_pi_attached

read_config_file

detect_os

migration_helper

configure_hello_video_args


detect_wfb_primary_band

auto_frequency_select

if [ "${FORCE_REALTEK_TELEMETRY_DATA_FRAME}" == "Y" ]; then
    export FORCE_REALTEK_TELEMETRY_DATA_FRAME=1
else
    export FORCE_REALTEK_TELEMETRY_DATA_FRAME=0
fi

echo "-------------------------------------"
echo "SETTINGS FILE: $CONFIGFILE"
echo "-------------------------------------"
if [ "$TTY" == "/dev/tty1" ]; then
    echo "Using settings file $CONFIGFILE"
    qstatus "Using settings file $CONFIGFILE" 5
fi

#
# Set the wifi parameters based on the selected datarate
#
datarate_to_wifi_settings

if [ "$CAM" == "0" ]; then
    #
    # For debugging viewing the different TTY consoles can be useful. Set in settings
    #
    chvt $TTY_CONSOLE

    #
    # Set the approperiate display font size
    #
    set_font_for_resolution

    #
    # Set the specififc video player based on the FPS
    #
    set_video_player_based_fps
fi	


# Fixed video values
VIDEO_UDP_BLOCKSIZE=1024
TELEMETRY_UDP_BLOCKSIZE=128

RELAY_VIDEO_BLOCKS=8
RELAY_VIDEO_FECS=4
RELAY_VIDEO_BLOCKLENGTH=1024

RSSI_UDP_PORT=5003

#
# Set telemetry settings based on telemetry type
#
get_telemetry_settings

#
# Configure CTS
#
set_cts_protection


#
# Flight controller and telemetry settings
#
FC_TELEMETRY_STTY_OPTIONS="-icrnl -ocrnl -imaxbel -opost -isig -icanon -echo -echoe -ixoff -ixon"
EXTERNAL_TELEMETRY_SERIALPORT_GROUND_STTY_OPTIONS="-icrnl -ocrnl -imaxbel -opost -isig -icanon -echo -echoe -ixoff -ixon"
TELEMETRY_OUTPUT_SERIALPORT_GROUND_STTY_OPTIONS="-icrnl -ocrnl -imaxbel -opost -isig -icanon -echo -echoe -ixoff -ixon"


#
# Include individual scripts to pull in the functions for each major process
#
source tx_rx_functions.sh
source osd_tx_rx_functions.sh
source uplink_functions.sh


#
# These are only used on the ground side, so don't even source them
#
if [ "$CAM" == "0" ]; then
    source rc_tx_rx_functions.sh	
    source rssi_rx_functions.sh
    source screenshot_functions.sh
    source alive_functions.sh
    source video_save_functions.sh
    source tether_functions.sh
    source hotspot_functions.sh
fi	


#
# This is the main process control/setup of Open.HD
#
# The various parts of the system are run on different TTYs to create a sort of parallel startup, while allowing
# people to directly view their output. Most of the processes can be moved to systemd once we convert them to
# read their own settings from the settings file. Some things should remain in bash, but only very high level
# configuration decisions if they can't be done any other way.
#

#
# Setup consoles that run on BOTH air and ground
#
case $TTY in
    /dev/tty1)
        MAIN_TX_RX_FUNCTION
    ;;
    /dev/tty2)
        MAIN_OSD_TX_RX_FUNCTION
    ;;
    /dev/tty10)
        MAIN_UPLINK_FUNCTION
    ;;
    /dev/tty11) 
        echo "================== eth0 DHCP client (tty11) ==========================="
        
        #
        # TODO: Move this to a separate script
        #

        if [ "$CAM" != "0" ] && [ "$DEBUG" == "Y" ] && [ "$SecondaryCamera" != "IP" ] || [ "$CAM" == "0" ]; then
    
            if [ "$CAM" == "0" ]; then
                OHDHOSTNAME="openhd-ground"
            else
                OHDHOSTNAME="openhd-air"
            fi
        
            #
            # If ethernet hotspot is disabled, we allow the ethernet interface to be used for connecting
            # to a normal LAN. This requires a router or something else handing out DHCP addresses, the
            # ground station only does that when ethernet hotspot is enabled
            #
            if [ "$ETHERNET_HOTSPOT" == "N" ]; then

                ip link set dev eth0 up
                
                while ! [ "$(cat /sys/class/net/eth0/operstate)" = "up" ] 
                do
                    sleep 1
                done
                
                if cat /sys/class/net/eth0/carrier | nice grep -q 1; then
                    echo "Ethernet connection detected"
                    qstatus "Ethernet connection detected" 5

                    CARRIER=1
                    
                    if nice pump -i eth0 --no-ntp -h $OHDHOSTNAME; then
                        ETHCLIENTIP=`ip addr show eth0 | grep -Po 'inet \K[\d.]+'`
                    
                        if [ "$ENABLE_QOPENHD" == "Y" ]; then
                            qstatus "Ethernet IP: $ETHCLIENTIP" 5
                        else
                            wbc_status "Ethernet IP: $ETHCLIENTIP" 7 55 0 &
                        fi

                        ping -n -q -c 1 1.1.1.1
                    else
                        ps -ef | nice grep "pump -i eth0" | nice grep -v grep | awk '{print $2}' | xargs kill -9

                        nice ifconfig eth0 down
                        
                        echo "DHCP failed"
                        qstatus "DHCP failed" 3
                        
                        killall wbc_status > /dev/null 2>&1
                        
                        if [ "$ENABLE_QOPENHD" == "Y" ]; then
                            qstatus "ERROR: Could not acquire IP via DHCP!" 5
                        else
                            wbc_status "ERROR: Could not acquire IP via DHCP!" 7 55 0 &
                        fi
                    fi
                else
                    echo "No ethernet connection detected"
                    qstatus "No ethernet connection detected" 5
                fi
            else
                echo "Ethernet Hotspot enabled, doing nothing"
                qstatus "Ethernet hotspot enabled" 5
            fi
            sleep 365d
        fi
    ;;
    /dev/tty12) 
        #
        # TTY reserved for local interactive login. You can switch to this one at any time with Ctrl-Alt-F12
        #
        # Note: currently QOpenHD requires using just Alt-F12 instead, due to what is possibly a bug in Qt
        #
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


#
# Setup consoles that run ONLY on GroundPi
#
if [ "$CAM" == "0" ]; then
    case $TTY in
        /dev/tty3)
            MAIN_RC_TX_RX_FUNCTION
        ;;
        /dev/tty4)
            MAIN_RSSI_RX_FUNCTION		
        ;;
        /dev/tty5)
            MAIN_SCREENSHOT_FUNCTION
        ;;
        /dev/tty6)
            MAIN_VIDEO_SAVE_FUNCTION	
        ;;
        /dev/tty7)
            MAIN_TETHER_FUNCTION
        ;;
        /dev/tty8)
            MAIN_HOTSPOT_FUNCTION
        ;;
        /dev/tty9)
            MAIN_ALIVE_FUNCTION
        ;;
        *) 
            #
            # All other TTYs used for interactive login and debugging
            #
            if [ "$CAM" == "0" ]; then
                echo "Welcome to OpenHD (ground) - the /boot filesystem is now read-write, type 'ro' to switch the boot filesystem back to read-only"
                
                #
                # Double to ensure the remount takes effect
                #
                rw
                rw
            else
                echo "Welcome to OpenHD (air) - the /boot filesystem is now read-write, type 'ro' to switch the boot filesystem back to read-only"
                
                rw
                rw
            fi
        ;;
    esac
fi
