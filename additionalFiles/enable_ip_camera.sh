#IP Camera Setup

rm -Rf /etc/NetworkManager/system-connections/ohd_ip_eth*
#ethernet hotspot with DHCP server on 192.168.2.1
sudo nmcli con add type ethernet con-name "ohd_ip_eth_hotspot" ipv4.method shared ifname eth0 ipv4.addresses 192.168.2.1/24 gw4 192.168.2.1
sudo nmcli con add type ethernet ifname eth0 con-name ohd_ip_eth_hotspot autoconnect no
sudo nmcli con modify ohd_ip_eth_hotspot ipv4.method shared ifname eth0 ipv4.addresses 192.168.2.1/24 gw4 192.168.2.1

echo "starting pipeline for the primary camera"
#pipe camera 1 to openhd uncommend below

#sudo gst-launch-1.0 rtspsrc location= rtsp://admin:admin@192.168.2.176:554/stream=0  latency=0 ! rtph264depay ! h264parse config-interval=-1 ! rtph264pay mtu=1024 ! udpsink port=5500 host=127.0.0.1
