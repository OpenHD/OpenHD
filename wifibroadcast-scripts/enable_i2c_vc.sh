#!/bin/bash

#
# If /boot/i2c_vc exists, we check to see if i2c_vc is already enabled in
# config.txt, and if not we enable it and reboot. 
#
# This should not add more than a few seconds to the air boot time, and 
# only needs to be done once, but IMX290 and other 3rd party cameras won't 
# work without it
#
# This should probably be called in early_init.sh or openhd-system
#
function autoenable_i2c_vc {
    if [ -e "/boot/i2c_vc" ]; then
        I2C_VC_ENABLED=$(cat /boot/config.txt | grep "^dtparam=i2c_vc" | wc -l)
        
        if [ ${I2C_VC_ENABLED} == "0" ]; then
        
            I2C_VC_DISABLED=$(cat /boot/config.txt | grep "^#dtparam=i2c_vc" | wc -l)
            
            mount -o remount,rw /boot
        
            if [ ${I2C_VC_DISABLED} == "1" ]; then
                #
                # Present but disabled, we can use sed it to enable it in-place
                #
                sed -i 's/^#dtparam=i2c_vc.*/dtparam=i2c_vc=on/g' /boot/config.txt
            else
                #
                # It's not in the file at all, so we'll have to add it manually, but at the end of the
                # file with an [all] section to ensure that the setting applies to ever model in case
                # the config.txt file has sections like [pi3] [pi0] at the end.
                #
                echo "[all]" >> /boot/config.txt
                echo "dtparam=i2c_vc=on" >> /boot/config.txt
            fi
        
            mount -o remount,ro /boot
        
            reboot
        fi
    fi
}
