function lte_function {  

    #
    # Need a first run type of tracker. 
    #
    # Delete the below file in /boot to force install to rerun
    #
    if test -f "/boot/zerotier-install-tracker.txt"; then
        echo "Zerotier already attempted to install..."
        qstatus "Zerotier already attempted to install..." 5
    else

        #
        # On first run need to give lte stick time to establish data connection so that install can be completed
        #
        sleep 25

        #
        # Finish installation of ZeroTier
        #
        # In builder it hangs during cross compile while generating token
        #
        wget -O z.sh https://install.zerotier.com/
        chmod +x z.sh
        ./z.sh 
        sudo cat /var/lib/zerotier-one/authtoken.secret >>.zeroTierOneAuthToken
        chmod 0600 .zeroTierOneAuthToken
        sudo apt install lsof
        sudo lsof | grep zero | grep LISTEN
        zerotier-cli status
        zerotier-cli info
        sleep 1
        zerotier-cli join $ZT_NETWORK
        sleep 1

        #
        # In the future we only want zerotier to start here instead of at boot
        #
        sudo systemctl stop zerotier-one
        sudo systemctl disable zerotier-one
        sudo mount -o remount,rw /boot
        sudo touch /boot/zerotier-install-tracker.txt
        sudo mount -o remount,ro /boot

        echo "zerotier-install-tracker.txt created... Install will not be attempted again.."
    fi	

    #
    # Find ID of wbc NIC
    #
    local PI_NIC=`ls /sys/class/net/ | nice grep -v eth0 | nice grep -v lo | nice grep -v usb | nice grep -v intwifi | nice grep -v wlan | nice grep -v relay | nice grep -v wifihotspot | nice grep -v eth1`

    #
    # Replace white space with comma for case where there are multiple NIC for wbc
    #
    PI_NIC=$(echo $PI_NIC | sed 's/ /","/g')

    echo "PI_NIC to be blacklisted for zerotier=$PI_NIC"
    qstatus "NIC to be blacklisted for zerotier: $PI_NIC" 5


    #
    # Blacklist all wbc connections except our ZeroTier LTE stick
    #
    echo '{"settings": {"interfacePrefixBlacklist": [ "eth0","wifihotspot0","'"$PI_NIC"'","lo" ],"allowTcpFallbackRelay": false}}' > /var/lib/zerotier-one/local.conf

    #
    # Start ZeroTier
    #
    qstatus "Stating ZeroTier" 5
    sudo systemctl start zerotier-one
    
}


