# NOTE: By default, you do not need to do any camera scripting in OpenHD !
# Only for advanced users !
# The custom unmanaged camera service executes this script with root when enabled.

# For reference, here again: To use this functionality, set either primary or secondary cam to custom unmanaged -
# after that, you can pipe rtp h264 data to localhost::5500 (primary) / localhost:5501 (secondary) via gstreamer pipeline(s)
# (h265,mjpeg if you set the custom unmanaged cam to h265,mjpeg via qopenhd).
# but you loose functionalities like variable bitrate, changing the resolution via qopenhd and more.

# Example setup IP camera as primary camera.
# 1) Setup rpi ethernet as dhcpcd provider
# 2) launch a pipeline streaming data via gstreamer to openhd for transmission

setup_and_stream_ip_cam_openipc(){
  # 1) dhcpcd provider if it doesn't exist already
  FILE_NM_CONNECTION = /etc/NetworkManager/system-connections/ohd_ip_eth_hotspot.nmconnection
  if test -f "$FILE_NM_CONNECTION"; then
    echo "ip cam hotspot already exists"
  else
    # create via nmcli
    echo "creating ip cam hotspot"
    #ethernet hotspot with DHCP server on 192.168.2.1
    sudo nmcli con add type ethernet con-name "ohd_ip_eth_hotspot" ipv4.method shared ifname eth0 ipv4.addresses 192.168.2.1/24 gw4 192.168.2.1
    sudo nmcli con add type ethernet ifname eth0 con-name ohd_ip_eth_hotspot autoconnect no
    sudo nmcli con modify ohd_ip_eth_hotspot ipv4.method shared ifname eth0 ipv4.addresses 192.168.2.1/24 gw4 192.168.2.1
  fi
  # 2) start streaming
  sudo gst-launch-1.0 rtspsrc location= rtsp://admin:admin@192.168.2.176:554/stream=0  latency=0 ! rtph264depay ! h264parse config-interval=-1 ! rtph264pay mtu=1024 ! udpsink port=5500 host=127.0.0.1
}

# uncomment below for openipc ip cam
# setup_and_stream_ip_cam_openipc

echo "Doing nothing"
sleep 356d