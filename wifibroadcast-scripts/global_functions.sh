function detect_os {
    source /etc/os-release

    if [[ "${VERSION_ID}" == "10" ]]; then
        export OPENHD_VERSION="buster"
    elif [[ "${VERSION_ID}" == "11" ]]; then
        export OPENHD_VERSION="bullseye"
    else
        export OPENHD_VERSION="unknown"
    fi
}

function configure_hello_video_args {
    if [[ "$OPENHD_VERSION" == "buster" ]]; then
        export HELLO_VIDEO_ARGS="1"
    else
        export HELLO_VIDEO_ARGS="0"
    fi
}

function detect_memory {
    TOTAL_MEMORY=$(cat /proc/meminfo | grep 'MemTotal' | awk '{print $2}')
}
