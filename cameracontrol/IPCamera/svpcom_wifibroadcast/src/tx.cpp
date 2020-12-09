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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <time.h>
#include <sys/resource.h>
#include <pcap/pcap.h>
#include <assert.h>

#include <string>
#include <memory>
#include <vector>

extern "C"
{
#include "fec.h"
}

#include "wifibroadcast.hpp"
#include "tx.hpp"
int frame_type;

Transmitter::Transmitter(int k, int n, const string &keypair):  fec_k(k), fec_n(n), block_idx(0),
                                                                fragment_idx(0),
                                                                max_packet_size(0)
{
    fec_p = fec_new(fec_k, fec_n);

    block = new uint8_t*[fec_n];
    for(int i=0; i < fec_n; i++)
    {
        block[i] = new uint8_t[MAX_FEC_PAYLOAD];
    }

    FILE *fp;
    if((fp = fopen(keypair.c_str(), "r")) == NULL)
    {
        throw runtime_error(string_format("Unable to open %s: %s", keypair.c_str(), strerror(errno)));
    }
    if (fread(tx_secretkey, crypto_box_SECRETKEYBYTES, 1, fp) != 1) throw runtime_error(string_format("Unable to read tx secret key: %s", strerror(errno)));
    if (fread(rx_publickey, crypto_box_PUBLICKEYBYTES, 1, fp) != 1) throw runtime_error(string_format("Unable to read rx public key: %s", strerror(errno)));
    fclose(fp);

    make_session_key();
}

Transmitter::~Transmitter()
{
    for(int i=0; i < fec_n; i++)
    {
        delete block[i];
    }
    delete block;

    fec_free(fec_p);
}


void Transmitter::make_session_key(void)
{
    randombytes_buf(session_key, sizeof(session_key));
    session_key_packet.packet_type = WFB_PACKET_KEY;
    randombytes_buf(session_key_packet.session_key_nonce, sizeof(session_key_packet.session_key_nonce));
    if (crypto_box_easy(session_key_packet.session_key_data, session_key, sizeof(session_key),
                        session_key_packet.session_key_nonce, rx_publickey, tx_secretkey) != 0)
    {
        throw runtime_error("Unable to make session key!");
    }
}


PcapTransmitter::PcapTransmitter(int k, int n, const string &keypair, uint8_t radio_port, const vector<string> &wlans) : Transmitter(k, n, keypair),
                                                                                                                        radio_port(radio_port),
                                                                                                                        current_output(0),
                                                                                                                        ieee80211_seq(0)
{
    char errbuf[PCAP_ERRBUF_SIZE];
    for(auto it=wlans.begin(); it!=wlans.end(); it++)
    {
        pcap_t *p = pcap_create(it->c_str(), errbuf);
        if (p == NULL){
            throw runtime_error(string_format("Unable to open interface %s in pcap: %s", it->c_str(), errbuf));
        }
        if (pcap_set_snaplen(p, 4096) !=0) throw runtime_error("set_snaplen failed");
        if (pcap_set_promisc(p, 1) != 0) throw runtime_error("set_promisc failed");
        //if (pcap_set_rfmon(p, 1) !=0) throw runtime_error("set_rfmon failed");
        if (pcap_set_timeout(p, -1) !=0) throw runtime_error("set_timeout failed");
        //if (pcap_set_buffer_size(p, 2048) !=0) throw runtime_error("set_buffer_size failed");
        if (pcap_activate(p) !=0) throw runtime_error(string_format("pcap_activate failed: %s", pcap_geterr(p)));
        //if (pcap_setnonblock(p, 1, errbuf) != 0) throw runtime_error(string_format("set_nonblock failed: %s", errbuf));

        ppcap.push_back(p);
    }
}


void PcapTransmitter::inject_packet(const uint8_t *buf, size_t size)
{
    uint8_t txbuf[MAX_PACKET_SIZE];
    uint8_t *p = txbuf;

    assert(size <= MAX_FORWARDER_PACKET_SIZE);

    // radiotap header
    memcpy(p, radiotap_header, sizeof(radiotap_header));
    p += sizeof(radiotap_header);

    // ieee80211 header
    if(frame_type == 0 || frame_type == 1)
    {
    	memcpy(p, ieee80211_header, sizeof(ieee80211_header));
    }
    if(frame_type == 2)
    {
        memcpy(p, ieee80211_header_rts, sizeof(ieee80211_header_rts));
    }
  
    p[SRC_MAC_LASTBYTE] = radio_port;
    p[DST_MAC_LASTBYTE] = radio_port;
    p[FRAME_SEQ_LB] = ieee80211_seq & 0xff;
    p[FRAME_SEQ_HB] = (ieee80211_seq >> 8) & 0xff;
    ieee80211_seq += 16;
  
   
    if(frame_type == 0 || frame_type == 1)
    {
	      p += sizeof(ieee80211_header);
    }
    if(frame_type == 2)
    {
	      p += sizeof(ieee80211_header_rts);
    }

    // FEC data
    memcpy(p, buf, size);
    p += size;

    if (pcap_inject(ppcap[current_output], txbuf, p - txbuf) != p - txbuf)
    {
        throw runtime_error(string_format("Unable to inject packet"));
    }
}

PcapTransmitter::~PcapTransmitter()
{
    for(auto it=ppcap.begin(); it != ppcap.end(); it++){
        pcap_close(*it);
    }
}


void Transmitter::send_block_fragment(size_t packet_size)
{
    uint8_t ciphertext[MAX_FORWARDER_PACKET_SIZE];
    wblock_hdr_t *block_hdr = (wblock_hdr_t*)ciphertext;
    long long unsigned int ciphertext_len;

    assert(packet_size <= MAX_FEC_PAYLOAD);

    block_hdr->packet_type = WFB_PACKET_DATA;
    block_hdr->nonce = htobe64(((block_idx & BLOCK_IDX_MASK) << 8) + fragment_idx);

    // encrypted payload
    crypto_aead_chacha20poly1305_encrypt(ciphertext + sizeof(wblock_hdr_t), &ciphertext_len,
                                         block[fragment_idx], packet_size,
                                         (uint8_t*)block_hdr, sizeof(wblock_hdr_t),
                                         NULL, (uint8_t*)(&(block_hdr->nonce)), session_key);

    inject_packet(ciphertext, sizeof(wblock_hdr_t) + ciphertext_len);
}

void Transmitter::send_session_key(void)
{
    //fprintf(stderr, "Announce session key\n");
    inject_packet((uint8_t*)&session_key_packet, sizeof(session_key_packet));
}

void Transmitter::send_packet(const uint8_t *buf, size_t size)
{
    wpacket_hdr_t packet_hdr;
    assert(size <= MAX_PAYLOAD_SIZE);

    packet_hdr.packet_size = htobe16(size);
    memset(block[fragment_idx], '\0', MAX_FEC_PAYLOAD);
    memcpy(block[fragment_idx], &packet_hdr, sizeof(packet_hdr));
    memcpy(block[fragment_idx] + sizeof(packet_hdr), buf, size);
    send_block_fragment(sizeof(packet_hdr) + size);
    max_packet_size = max(max_packet_size, sizeof(packet_hdr) + size);
    fragment_idx += 1;

    if (fragment_idx < fec_k)  return;

    fec_encode(fec_p, (const uint8_t**)block, block + fec_k, max_packet_size);
    while (fragment_idx < fec_n)
    {
        send_block_fragment(max_packet_size);
        fragment_idx += 1;
    }
    block_idx += 1;
    fragment_idx = 0;
    max_packet_size = 0;

    // Generate new session key after MAX_BLOCK_IDX blocks
    if (block_idx > MAX_BLOCK_IDX)
    {
        make_session_key();
        send_session_key();
        block_idx = 0;
    }
}

void video_source(shared_ptr<Transmitter> &t, vector<int> &tx_fd)
{
    int nfds = tx_fd.size();
    struct pollfd fds[nfds];
    memset(fds, '\0', sizeof(fds));

    int i = 0;
    for(auto it=tx_fd.begin(); it != tx_fd.end(); it++, i++)
    {
        int fd = *it;
        if(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) < 0)
        {
            throw runtime_error(string_format("Unable to set socket into nonblocked mode: %s", strerror(errno)));
        }

        fds[i].fd = fd;
        fds[i].events = POLLIN;
    }

    uint64_t session_key_announce_ts = 0;

    for(;;)
    {
        int rc = poll(fds, nfds, -1);

        if (rc < 0){
            if (errno == EINTR || errno == EAGAIN) continue;
            throw runtime_error(string_format("poll error: %s", strerror(errno)));
        }

        if (rc == 0) continue;  // timeout expired

        for(i = 0; i < nfds; i++)
        {
            // some events detected
            if (fds[i].revents & (POLLERR | POLLNVAL))
            {
                throw runtime_error(string_format("socket error: %s", strerror(errno)));
            }

            if (fds[i].revents & POLLIN)
            {
                uint8_t buf[MAX_PAYLOAD_SIZE];
                ssize_t rsize;
                int fd = tx_fd[i];

                t->select_output(i);
                while((rsize = recv(fd, buf, sizeof(buf), 0)) >= 0)
                {
                    uint64_t cur_ts = get_time_ms();
                    if (cur_ts >= session_key_announce_ts)
                    {
                        // Announce session key
                        t->send_session_key();
                        session_key_announce_ts = cur_ts + SESSION_KEY_ANNOUNCE_MSEC;
                    }
                    t->send_packet(buf, rsize);
                }
                if(errno != EWOULDBLOCK) throw runtime_error(string_format("Error receiving packet: %s", strerror(errno)));
            }
        }
    }
}


int main(int argc, char * const *argv)
{
    int opt;
    uint8_t k=8, n=12, radio_port=1;
    int udp_port=5600;

    int bandwidth = 40;
    int short_gi = 1;
    int stbc = 1;
    int mcs_index = 1;
    frame_type=0;

    string keypair = "tx.key";

    while ((opt = getopt(argc, argv, "K:k:n:u:r:p:B:G:S:M:t:")) != -1) {
        switch (opt) {
        case 'K':
            keypair = optarg;
            break;
        case 'k':
            k = atoi(optarg);
            break;
        case 'n':
            n = atoi(optarg);
            break;
        case 'u':
            udp_port = atoi(optarg);
            break;
        case 'p':
            radio_port = atoi(optarg);
            break;
        case 'B':
            bandwidth = atoi(optarg);
            break;
        case 'G':
            short_gi = (optarg[0] == 's' || optarg[0] == 'S') ? 1 : 0;
            break;
        case 'S':
            stbc = atoi(optarg);
            break;
        case 'M':
            mcs_index = atoi(optarg);
            break;
        case 't':
	          frame_type = atoi(optarg);
            break;
        default: /* '?' */
        show_usage:
            fprintf(stderr, "Usage: %s [-K tx_key] [-k RS_K] [-n RS_N] [-u udp_port] [-p radio_port] [-B bandwidth] [-G guard_interval] [-S stbc] [-M mcs_index] interface1 [interface2] ...\n",
                    argv[0]);
            fprintf(stderr, "Default: K='%s', k=%d, n=%d, udp_port=%d, radio_port=%d bandwidth=%d guard_interval=%s stbc=%d mcs_index=%d\n",
                    keypair.c_str(), k, n, udp_port, radio_port, bandwidth, short_gi ? "short" : "long", stbc, mcs_index);
            fprintf(stderr, "Radio MTU: %lu\n", (unsigned long)MAX_PAYLOAD_SIZE);
            fprintf(stderr, "WFB version " WFB_VERSION "\n");
            exit(1);
        }
    }

    if (optind >= argc) {
        goto show_usage;
    }

    // Set flags in radiotap header
    {
        uint8_t flags = 0;
        switch(bandwidth) {
        case 20:
            flags |= IEEE80211_RADIOTAP_MCS_BW_20;
            break;
        case 40:
            flags |= IEEE80211_RADIOTAP_MCS_BW_40;
            break;
        default:
            fprintf(stderr, "Unsupported bandwidth: %d\n", bandwidth);
            exit(1);
        }

        if(short_gi)
        {
            flags |= IEEE80211_RADIOTAP_MCS_SGI;
        }

        switch(stbc) {
        case 0:
            break;
        case 1:
            flags |= (IEEE80211_RADIOTAP_MCS_STBC_1 << IEEE80211_RADIOTAP_MCS_STBC_SHIFT);
            break;
        case 2:
            flags |= (IEEE80211_RADIOTAP_MCS_STBC_2 << IEEE80211_RADIOTAP_MCS_STBC_SHIFT);
            break;
        case 3:
            flags |= (IEEE80211_RADIOTAP_MCS_STBC_3 << IEEE80211_RADIOTAP_MCS_STBC_SHIFT);
            break;
        default:
            fprintf(stderr, "Unsupported STBC type: %d\n", stbc);
            exit(1);
        }

        radiotap_header[MCS_FLAGS_OFF] = flags;
        radiotap_header[MCS_IDX_OFF] = mcs_index;
    }
    try
    {
        vector<int> tx_fd;
        vector<string> wlans;
        for(int i = 0; optind + i < argc; i++)
        {
            int fd = open_udp_socket_for_rx(udp_port + i);
            fprintf(stderr, "Listen on %d for %s\n", udp_port + i, argv[optind + i]);
            tx_fd.push_back(fd);
            wlans.push_back(string(argv[optind + i]));
        }

#ifdef DEBUG_TX
        shared_ptr<Transmitter>t = shared_ptr<UdpTransmitter>(new UdpTransmitter(k, n, keypair, "127.0.0.1", 5601 + i));
#else
        shared_ptr<Transmitter>t = shared_ptr<PcapTransmitter>(new PcapTransmitter(k, n, keypair, radio_port, wlans));
#endif

        video_source(t, tx_fd);
    }catch(runtime_error &e)
    {
        fprintf(stderr, "Error: %s\n", e.what());
        exit(1);
    }
    return 0;
}
