// Copyright (C) 2017, 2018 Vasily Evseenko <svpcom@p2ptech.org>

/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 3.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#ifndef __WIFIBROADCAST_HPP__
#define __WIFIBROADCAST_HPP__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <resolv.h>
#include <string.h>
#include <utime.h>
#include <unistd.h>
#include <getopt.h>
#include <pcap.h>
#include <endian.h>
#include <fcntl.h>
#include <time.h>
#include <sys/mman.h>
#include <sodium.h>
#include <endian.h>

#define MAX_PACKET_SIZE 1510
#define MAX_RX_INTERFACES 8

using namespace std;

extern string string_format(const char *format, ...);

/* this is the template radiotap header we send packets out with */


#define IEEE80211_RADIOTAP_MCS_HAVE_BW          0x01
#define IEEE80211_RADIOTAP_MCS_HAVE_MCS         0x02
#define IEEE80211_RADIOTAP_MCS_HAVE_GI          0x04
#define IEEE80211_RADIOTAP_MCS_HAVE_FMT         0x08

#define         IEEE80211_RADIOTAP_MCS_BW_20    0
#define         IEEE80211_RADIOTAP_MCS_BW_40    1
#define         IEEE80211_RADIOTAP_MCS_BW_20L   2
#define         IEEE80211_RADIOTAP_MCS_BW_20U   3
#define IEEE80211_RADIOTAP_MCS_SGI              0x04
#define IEEE80211_RADIOTAP_MCS_FMT_GF           0x08

#define IEEE80211_RADIOTAP_MCS_HAVE_STBC  0x20
#define	IEEE80211_RADIOTAP_MCS_STBC_MASK  0x60
#define	IEEE80211_RADIOTAP_MCS_STBC_1  1
#define	IEEE80211_RADIOTAP_MCS_STBC_2  2
#define	IEEE80211_RADIOTAP_MCS_STBC_3  3
#define	IEEE80211_RADIOTAP_MCS_STBC_SHIFT 5

#define MCS_KNOWN (IEEE80211_RADIOTAP_MCS_HAVE_MCS | IEEE80211_RADIOTAP_MCS_HAVE_BW | IEEE80211_RADIOTAP_MCS_HAVE_GI | IEEE80211_RADIOTAP_MCS_HAVE_STBC) // | IEEE80211_RADIOTAP_MCS_HAVE_FMT)

// Default is MCS#1 -- QPSK 1/2 40MHz SGI -- 30 Mbit/s
// MCS_FLAGS = (IEEE80211_RADIOTAP_MCS_BW_40 | IEEE80211_RADIOTAP_MCS_SGI | (IEEE80211_RADIOTAP_MCS_STBC_1 << IEEE80211_RADIOTAP_MCS_STBC_SHIFT) ) // | IEEE80211_RADIOTAP_MCS_FMT_GF)

static uint8_t radiotap_header[]  __attribute__((unused)) = {
    0x00, 0x00, // <-- radiotap version
    0x0d, 0x00, // <- radiotap header length
    0x00, 0x80, 0x08, 0x00, // <-- radiotap present flags:  RADIOTAP_TX_FLAGS + RADIOTAP_MCS
    0x08, 0x00,  // RADIOTAP_F_TX_NOACK
    MCS_KNOWN , 0x00, 0x00 // bitmap, flags, mcs_index
};

// offset of MCS_FLAGS and MCS index
#define MCS_FLAGS_OFF 11
#define MCS_IDX_OFF 12

//the last byte of the mac address is recycled as a port number
#define SRC_MAC_LASTBYTE 15
#define DST_MAC_LASTBYTE 21
#define FRAME_SEQ_LB 22
#define FRAME_SEQ_HB 23

static uint8_t ieee80211_header[] __attribute__((unused)) = {
    0x08, 0x01, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x13, 0x22, 0x33, 0x44, 0x55, 0x66,
    0x13, 0x22, 0x33, 0x44, 0x55, 0x66,
    0x00, 0x00,  // seq num << 4 + fragment num
};

static uint8_t ieee80211_header_rts[] __attribute__((unused)) = {
    0xb4, 0x01, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x13, 0x22, 0x33, 0x44, 0x55, 0x66,
    0x13, 0x22, 0x33, 0x44, 0x55, 0x66,
   0x00, 0x00,  // seq num << 4 + fragment num
};

/*
 Wifibroadcast protocol:

 radiotap_header
   ieee_80211_header
     wblock_hdr_t   { packet_type, nonce = (block_idx << 8 + fragment_idx) }
       wpacket_hdr_t  { packet_size }  #
         data                          #
                                       +-- encrypted

 */

// nonce:  56bit block_idx + 8bit fragment_idx

#define BLOCK_IDX_MASK ((1LLU << 56) - 1)
#define MAX_BLOCK_IDX ((1LLU << 55) - 1)


#define WFB_PACKET_DATA 0x1
#define WFB_PACKET_KEY 0x2

#define SESSION_KEY_ANNOUNCE_MSEC 1000
#define RX_ANT_MAX  4

// Header for forwarding raw packets from RX host to Aggregator in UDP packets
typedef struct {
    uint8_t wlan_idx;
    uint8_t antenna[RX_ANT_MAX]; //RADIOTAP_ANTENNA, list of antenna idx, 0xff for unused slot
    int8_t rssi[RX_ANT_MAX]; //RADIOTAP_DBM_ANTSIGNAL, list of rssi for corresponding antenna idx
} __attribute__ ((packed)) wrxfwd_t;

// Network packet headers. All numbers are in network (big endian) format
// Encrypted packets can be either session key or data packet.

// Session key packet

typedef struct {
    uint8_t packet_type;
    uint8_t session_key_nonce[crypto_box_NONCEBYTES];  // random data
    uint8_t session_key_data[crypto_aead_chacha20poly1305_KEYBYTES + crypto_box_MACBYTES]; // encrypted session key
} __attribute__ ((packed)) wsession_key_t;

// Data packet. Embed FEC-encoded data

typedef struct {
    uint8_t packet_type;
    uint64_t nonce;  // big endian, nonce = block_idx << 8 + fragment_idx
}  __attribute__ ((packed)) wblock_hdr_t;

// Plain data packet after FEC decode

typedef struct {
    uint16_t packet_size; // big endian
}  __attribute__ ((packed)) wpacket_hdr_t;

#define MAX_PAYLOAD_SIZE (MAX_PACKET_SIZE - sizeof(radiotap_header) - sizeof(ieee80211_header) - sizeof(wblock_hdr_t) - crypto_aead_chacha20poly1305_ABYTES - sizeof(wpacket_hdr_t))
#define MAX_FEC_PAYLOAD  (MAX_PACKET_SIZE - sizeof(radiotap_header) - sizeof(ieee80211_header) - sizeof(wblock_hdr_t) - crypto_aead_chacha20poly1305_ABYTES)
#define MAX_FORWARDER_PACKET_SIZE (MAX_PACKET_SIZE - sizeof(radiotap_header) - sizeof(ieee80211_header))

int open_udp_socket_for_rx(int port);
uint64_t get_time_ms(void);

#endif
