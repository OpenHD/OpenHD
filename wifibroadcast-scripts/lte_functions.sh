function lte_function {  

	PI_NIC = `ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot | nice grep -v $NIC_BLACKLIST`

	sudo touch /var/lib/zerotier-one/local.conf

	echo `{
        	"settings": {
                	"interfacePrefixBlacklist": [ "eth0","wifihotspot0","$PI_NIC","lo" ],
                	"allowTcpFallbackRelay": false
        		}
		}` >> /var/lib/zerotier-one/local.conf


	sudo systemctl start zerotier-one

	zerotier-cli join $ZEROTIER


}
