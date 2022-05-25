function rx_function {
    #
    # Start audio and remote settings
    #
    if [ "$IsAudioTransferEnabled" == "1" ]; then
        echo "Audio enabled"
        qstatus "Audio enabled" 5

        amixer cset numid=3 $DefaultAudioOut

        /usr/local/share/RemoteSettings/Ground/AudioPlayback.sh &
        /usr/local/share/RemoteSettings/Ground/RxAudio.sh &
    fi
}
