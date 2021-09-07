function camera_setup {
    #
    # Look for a VEYE camera by probing the i2c bus. Note that this *requires*
    # i2c_vc to already be enabled or the bus won't even be available.
    #
    i2cdetect -y 0 0x3b 0x3b | grep  "30:                                  3b            "
    grepRet=$?
    if [[ $grepRet -eq 0 ]] ; then
        echo "VEYE camera detected"

        #
        # Signal to the rest of the system that a VEYE camera was detected
        #
        echo "1" > /tmp/veye
        VEYE="1"

        pushd /usr/local/share/veye-raspberrypi

        /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -r -f devid > /tmp/veyemipidevid
        /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -r -f hdver >> /tmp/veyemipidevid
        /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -r -f devid | grep "0x 6"
        grepRet=$?
        if [[ $grepRet -eq 0 ]] ; then
            echo "VEYE MIPI Detected"

            #
            # Load the settings based on the profile selected.
            #

            wdrmode=VEYE_MIPI${VEYE_MIPI_profile}_wdrmode
            denoise=VEYE_MIPI${VEYE_MIPI_profile}_denoise
            lowlight=VEYE_MIPI${VEYE_MIPI_profile}_lowlight
            agc=VEYE_MIPI${VEYE_MIPI_profile}_agc
            brightness=VEYE_MIPI${VEYE_MIPI_profile}_brightness
            aespeed1=VEYE_MIPI${VEYE_MIPI_profile}_aespeed1
            aespeed2=VEYE_MIPI${VEYE_MIPI_profile}_aespeed2
            contrast=VEYE_MIPI${VEYE_MIPI_profile}_contrast
            saturation=VEYE_MIPI${VEYE_MIPI_profile}_saturation
            sharppen1=VEYE_MIPI${VEYE_MIPI_profile}_sharppen1
            sharppen2=VEYE_MIPI${VEYE_MIPI_profile}_sharppen2
            wdrtargetbr=VEYE_MIPI${VEYE_MIPI_profile}_wdrtargetbr
            wdrbtargetbr=VEYE_MIPI${VEYE_MIPI_profile}_wdrbtargetbr
            daynightmode=VEYE_MIPI${VEYE_MIPI_profile}_daynightmode
            mshutter=VEYE_MIPI${VEYE_MIPI_profile}_mshutter
            wbmode=VEYE_MIPI${VEYE_MIPI_profile}_wbmode
            mwbgain1=VEYE_MIPI${VEYE_MIPI_profile}_mwbgain1
            mwbgain2=VEYE_MIPI${VEYE_MIPI_profile}_mwbgain2
            yuvseq=VEYE_MIPI${VEYE_MIPI_profile}_yuvseq

            #
            # Configure the camera's ISP parameters
            #
            
            /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f wdrmode -p1 ${!wdrmode}> /tmp/veyelog
            /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f mirrormode -p1 $VEYE_MIPI_mirrormode >> /tmp/veyelog
            /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f denoise -p1 ${!denoise} >> /tmp/veyelog

            if [ "${!lowlight}" != "" ]; then
                /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f lowlight -p1 ${!lowlight} >> /tmp/veyelog
            else
                # turn it off by default to avoid framerate changing during flight
                /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f lowlight -p1 0x00 >> /tmp/veyelog
            fi

            if [ "${!agc}" != "" ]; then
                /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f agc -p1 ${!agc} >> /tmp/veyelog
            fi

            if [ "${!brightness}" != "" ]; then
                /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f brightness -p1 ${!brightness} >> /tmp/veyelog
            fi

            if [ "${!aespeed1}" != "" ]; then
                /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f aespeed  -p1 ${!aespeed1} -p2 ${!aespeed2} >> /tmp/veyelog
            fi

            if [ "${!contrast}" != "" ]; then
                /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f contrast -p1 ${!contrast} >> /tmp/veyelog
            fi

            if [ "${!saturation}" != "" ]; then
                /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f saturation -p1 ${!saturation} >> /tmp/veyelog
            fi

            if [ "${!sharppen1}" != "" ]; then
                /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f sharppen -p1 ${!sharppen1} -p2 ${!sharppen2} >> /tmp/veyelog
            fi

            if [ "${!wdrtargetbr}" != "" ]; then
                /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f wdrtargetbr -p1 ${!wdrtargetbr} >> /tmp/veyelog
            fi

            if [ "${!wdrbtargetbr}" != "" ]; then
                /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f wdrbtargetbr -p1 ${!wdrbtargetbr} >> /tmp/veyelog
            fi

            if [ "${!daynightmode}" != "" ]; then
                /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f daynightmode -p1 ${!daynightmode} >> /tmp/veyelog
            fi

            if [ "${!mshutter}" != "" ]; then
                /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f mshutter -p1 ${!mshutter} >> /tmp/veyelog
            fi

            if [ "${!wbmode}" != "" ]; then
                /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f wbmode -p1 ${!wbmode} >> /tmp/veyelog
            fi
            
            if [ "${!mwbgain1}" != "" ]; then
                /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f mwbgain -p1 ${!mwbgain1} -p2 ${!mwbgain2} >> /tmp/veyelog
            fi
            
            if [ "${!yuvseq}" != "" ]; then
                /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f yuvseq -p1 ${!yuvseq} >> /tmp/veyelog
            fi
        fi


        /usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -r -f devid > /tmp/veyecsdevid
        /usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -r -f firmwarever >> /tmp/veyecsdevid
        /usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -r -f devid | grep "0x 2"
        grepRet=$?
        if [[ $grepRet -eq 0 ]] ; then
            echo "VEYE CS Detected"

            #
            # Check the firmware version so we know what videofmt to use.
            # On Version >2.35 a decimal framerate is used and set via -p2
            #

            fwv=$(/usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -r -f firmwarever | grep -o -E "[0-9]+\.[0-9]+")
            echo 'firmware Version:'
            echo $fwv

            # Version to check against
            fw=2.35

            if [[ $fwv > $fw ]]; then
                /usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -w -f videofmt -p1 ${WIDTH} -p2 ${HEIGHT} -p4 ${FPS}.0
            else
                /usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -w -f videofmt -p1 ${WIDTH} -p2 ${HEIGHT} -p3 ${FPS}
            fi

            /usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -w -f imagedir -p1 $VEYE_CS_imagedir >> /tmp/veyelog

            if [ "${!daynightmode}" != "" ]; then
                /usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -w -f daynightmode -p1 ${!daynightmode} >> /tmp/veyelog
                
            if [ "${!hue}" != "" ]; then
                /usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -w -f hue -p1 ${!hue} >> /tmp/veyelog
                
            if [ "${!contrast}" != "" ]; then
                /usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -w -f contrast -p1 ${!contrast} >> /tmp/veyelog
                
            if [ "${!satu}" != "" ]; then
                /usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -w -f satu -p1 ${!satu} >> /tmp/veyelog
                                
            if [ "${!expmode}" != "" ]; then
                /usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -w -f expmode -p1 ${!expmode} >> /tmp/veyelog
                                              
            if [ "${!aetarget}" != "" ]; then
                /usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -w -f aetarget -p1 ${!aetarget} >> /tmp/veyelog
                                              
            if [ "${!aetime}" != "" ]; then
                /usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -w -f aetime -p1 ${!aetime} >> /tmp/veyelog
                                                              
            if [ "${!aeagc}" != "" ]; then
                /usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -w -f aeagc -p1 ${!aeagc} >> /tmp/veyelog
                                                                              
            if [ "${!metime}" != "" ]; then
                /usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -w -f metime -p1 ${!metime} >> /tmp/veyelog
                                                                                              
            if [ "${!meagain}" != "" ]; then
                /usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -w -f meagain -p1 ${!meagain} >> /tmp/veyelog
                                                                                                              
            if [ "${!medgain}" != "" ]; then
                /usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -w -f medgain -p1 ${!medgain} >> /tmp/veyelog
                                                                                                                              
            if [ "${!awbmode}" != "" ]; then
                /usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -w -f awbmode -p1 ${!awbmode} >> /tmp/veyelog
                                                                                                                                              
            if [ "${!mwbgain}" != "" ]; then
                /usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -w -f mwbgain -p1 ${!mwbgain} >> /tmp/veyelog
                                                                                                                                                              
            if [ "${!mwbcolortemp}" != "" ]; then
                /usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -w -f mwbcolortemp -p1 ${!mwbcolortemp} >> /tmp/veyelog
                                                                                                                                                                              
            if [ "${!yuvseq}" != "" ]; then
                /usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -w -f yuvseq -p1 ${!yuvseq} >> /tmp/veyelog
                
            
        fi

        popd
    fi
}