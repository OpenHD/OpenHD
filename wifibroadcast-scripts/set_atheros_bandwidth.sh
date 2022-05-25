#!/bin/bash

BANDWIDTH=$1

if [ "${BANDWIDTH}" == "10" ]; then
    /usr/local/bin/qstatus "Using 10MHz atheros channel bandwidth" 5

    echo 0xa > /sys/kernel/debug/ieee80211/phy0/ath9k_htc/chanbw
    echo 0xa > /sys/kernel/debug/ieee80211/phy1/ath9k_htc/chanbw
    echo 0xa > /sys/kernel/debug/ieee80211/phy2/ath9k_htc/chanbw
    echo 0xa > /sys/kernel/debug/ieee80211/phy3/ath9k_htc/chanbw
    echo 0xa > /sys/kernel/debug/ieee80211/phy4/ath9k_htc/chanbw
fi

if [ "${BANDWIDTH}" == "5" ]; then
    /usr/local/bin/qstatus "Using 5MHz atheros channel bandwidth" 5

    echo 5 > /sys/kernel/debug/ieee80211/phy0/ath9k_htc/chanbw
    echo 5 > /sys/kernel/debug/ieee80211/phy1/ath9k_htc/chanbw
    echo 5 > /sys/kernel/debug/ieee80211/phy2/ath9k_htc/chanbw
    echo 5 > /sys/kernel/debug/ieee80211/phy3/ath9k_htc/chanbw
    echo 5 > /sys/kernel/debug/ieee80211/phy4/ath9k_htc/chanbw
fi
