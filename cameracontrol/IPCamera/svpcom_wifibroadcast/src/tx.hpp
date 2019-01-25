// -*- C++ -*-
//
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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

class Transmitter
{
public:
    Transmitter(int k, int m, const string &keypair);
    virtual ~Transmitter();
    void send_packet(const uint8_t *buf, size_t size);
    void send_session_key(void);
    virtual void select_output(int idx) = 0;
protected:
    virtual void inject_packet(const uint8_t *buf, size_t size) = 0;

private:
    void send_block_fragment(size_t packet_size);
    void make_session_key(void);

    fec_t* fec_p;
    int fec_k;  // RS number of primary fragments in block
    int fec_n;  // RS total number of fragments in block
    uint64_t block_idx; //block_idx << 8 + fragment_idx = nonce (64bit)
    uint8_t fragment_idx;
    uint8_t** block;
    size_t max_packet_size;

    // tx->rx keypair
    uint8_t tx_secretkey[crypto_box_SECRETKEYBYTES];
    uint8_t rx_publickey[crypto_box_PUBLICKEYBYTES];
    uint8_t session_key[crypto_aead_chacha20poly1305_KEYBYTES];
    wsession_key_t session_key_packet;
};


class PcapTransmitter : public Transmitter
{
public:
    PcapTransmitter(int k, int m, const string &keypair, uint8_t radio_port, const vector<string> &wlans);
    virtual ~PcapTransmitter();
    virtual void select_output(int idx) { current_output = idx; }
private:
    virtual void inject_packet(const uint8_t *buf, size_t size);
    uint8_t radio_port;
    int current_output;
    uint16_t ieee80211_seq;
    vector<pcap_t*> ppcap;
};


class UdpTransmitter : public Transmitter
{
public:
    UdpTransmitter(int k, int m, const string &keypair, const string &client_addr, int client_port) : Transmitter(k, m, keypair)
    {
        sockfd = open_udp_socket(client_addr, client_port);
    }

    virtual ~UdpTransmitter()
    {
        close(sockfd);
    }

    virtual void select_output(int idx){}

private:
    virtual void inject_packet(const uint8_t *buf, size_t size)
    {
        wrxfwd_t fwd_hdr = { .wlan_idx = (uint8_t)(rand() % 2) };

        memset(fwd_hdr.antenna, 0xff, sizeof(fwd_hdr.antenna));
        memset(fwd_hdr.rssi, SCHAR_MIN, sizeof(fwd_hdr.rssi));

        fwd_hdr.antenna[0] = (uint8_t)(rand() % 2);
        fwd_hdr.rssi[0] = (int8_t)(rand() & 0xff);

        struct iovec iov[2] = {{ .iov_base = (void*)&fwd_hdr,
                                 .iov_len = sizeof(fwd_hdr)},
                               { .iov_base = (void*)buf,
                                 .iov_len = size }};

        struct msghdr msghdr = { .msg_name = NULL,
                                 .msg_namelen = 0,
                                 .msg_iov = iov,
                                 .msg_iovlen = 2,
                                 .msg_control = NULL,
                                 .msg_controllen = 0,
                                 .msg_flags = 0};

        sendmsg(sockfd, &msghdr, MSG_DONTWAIT);
    }

    int open_udp_socket(const string &client_addr, int client_port)
    {
        struct sockaddr_in saddr;
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd < 0) throw runtime_error(string_format("Error opening socket: %s", strerror(errno)));

        bzero((char *) &saddr, sizeof(saddr));
        saddr.sin_family = AF_INET;
        saddr.sin_addr.s_addr = inet_addr(client_addr.c_str());
        saddr.sin_port = htons((unsigned short)client_port);

        if (connect(fd, (struct sockaddr *) &saddr, sizeof(saddr)) < 0)
        {
            throw runtime_error(string_format("Connect error: %s", strerror(errno)));
        }
        return fd;
    }

    int sockfd;
};
