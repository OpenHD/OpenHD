#IP Camera Setup

rm -Rf /etc/NetworkManager/system-connections/ohd_ip_eth*
#ethernet hotspot with DHCP server on 192.168.2.1
sudo nmcli con add type ethernet con-name "ohd_ip_eth_hotspot" ipv4.method shared ifname eth0 ipv4.addresses 192.168.2.1/24 gw4 192.168.2.1
sudo nmcli con add type ethernet ifname eth0 con-name ohd_ip_eth_hotspot autoconnect no
sudo nmcli con modify ohd_ip_eth_hotspot ipv4.method shared ifname eth0 ipv4.addresses 192.168.2.1/24 gw4 192.168.2.1


#detect IP camera address
nmap -sn 192.168.2.0/24 | awk '/Nmap scan report/{ip=$5} ip != "192.168.2.1" && !seen[ip] {print ip; seen[ip]=1}' > /boot/IPCameraIP.txt
sed -i '1{/^$/d}' /boot/IPCameraIP.txt


echo "starting pipeline for the primary camera"
#pipe camera 1 to openhd uncommend below

#sudo gst-launch-1.0 rtspsrc location= rtsp://USERNAME:PASSWORD@STREAMURL  latency=0 ! rtph264depay ! h264parse config-interval=-1 ! rtph264pay mtu=1024 ! udpsink port=5500 host=127.0.0.1 &>/dev/null & disown

echo "starting pipeline for the secondary camera"
#pipe camera 2 to openhd uncommend below

#sudo gst-launch-1.0 videotestsrc ! x264enc ! rtph264pay ! udpsink host=localhost port=5501

