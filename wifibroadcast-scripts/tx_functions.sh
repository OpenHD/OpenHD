function tx_function {
    #
    # Look for a VEYE camera by probing the i2c bus. Note that this *requires*
    # i2c_vc to already be enabled or the bus won't even be available.
    #
    i2cdetect -y 0 | grep  "30: -- -- -- -- -- -- -- -- -- -- -- 3b -- -- -- --"
    grepRet=$?
    if [[ $grepRet -eq 0 ]] ; then
        echo "VEYE camera detected"

        #
        # Signal to the rest of the system that a VEYE camera was detected
        #
        echo "1" > /tmp/imx290
        IMX290="1"

        #
        # Configure the camera's ISP parameters
        #
        pushd /usr/local/share/veye-raspberrypi
        /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f wdrmode -p1 $IMX290_wdrmode > /tmp/imx290log
        /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f mirrormode -p1 $IMX290_mirrormode >> /tmp/imx290log
        /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f denoise -p1 $IMX290_denoise >> /tmp/imx290log

        if [ "${IMX290_lowlight}" != "" ]; then
            /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f lowlight -p1 ${IMX290_lowlight} >> /tmp/imx290log
        else
            # turn it off by default to avoid framerate changing during flight
            /usr/local/share/veye-raspberrypi/veye_mipi_i2c.sh -w -f lowlight -p1 0x00 >> /tmp/imx290log
        fi

        /usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -w -f imagedir -p1 $IMX307_imagedir >> /tmp/imx290log
        /usr/local/share/veye-raspberrypi/cs_mipi_i2c.sh -w -f videofmt -p1 ${WIDTH} -p2 ${HEIGHT} -p3 ${FPS}
        popd
    fi
    
    if [ "$IsAudioTransferEnabled" == "1" ]; then
        qstatus "Audio enabled" 5
        /usr/local/share/RemoteSettings/Air/AudioCapture.sh &
        /usr/local/share/RemoteSettings/Air/AudioTX.sh &
    fi
}
