function lte_function {  

	# TODO need a first run type of tracker.

	# On first run need to give lte stick time to establish data connection so that install can be completed
	sleep 25

	# FINISH INSTALL of ZeroTier. In builder it hangs during crosscompile generating token
	/home/pi/z.sh 
	sudo cat /var/lib/zerotier-one/authtoken.secret >>.zeroTierOneAuthToken
	chmod 0600 .zeroTierOneAuthToken
	sudo apt install lsof

	sleep 2

	# In the future we only want zerotier to start here instead of at boot
	sudo systemctl stop zerotier-one
	sudo systemctl disable zerotier-one

	sleep 2

	# Find id of wbc nic
	local PI_NIC=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot | nice grep -v $NIC_BLACKLIST | nice grep -v eth1`

echo "PI_NIC to be blacklisted for zerotier=$PI_NIC"

	# Blacklist all wbc connections except our zerotier lte stick
	echo '{"settings": {"interfacePrefixBlacklist": [ "eth0","wifihotspot0","'"$PI_NIC"'","lo" ],"allowTcpFallbackRelay": false}}' > /var/lib/zerotier-one/local.conf

	# Start zerotier
	sudo systemctl start zerotier-one

	sleep 2

	# This could possibly be part of install above...
	zerotier-cli join $ZEROTIER

}

