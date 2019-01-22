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

#include <assert.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/resource.h>
#include <pcap/pcap.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <limits.h>

extern "C"
{
#include "ieee80211_radiotap.h"
#include "fec.h"
}

#include <string>
#include <memory>

#include "wifibroadcast.hpp"
#include "rx.hpp"


Receiver::Receiver(const char *wlan, int wlan_idx, int radio_port, BaseAggregator *agg) : wlan_idx(wlan_idx), agg(agg)
{
    char errbuf[PCAP_ERRBUF_SIZE];

    ppcap = pcap_create(wlan, errbuf);

    if (ppcap == NULL){
        throw runtime_error(string_format("Unable to open interface %s in pcap: %s", wlan, errbuf));
    }

    if (pcap_set_snaplen(ppcap, 4096) !=0) throw runtime_error("set_snaplen failed");
    if (pcap_set_promisc(ppcap, 1) != 0) throw runtime_error("set_promisc failed");
    //if (pcap_set_rfmon(ppcap, 1) !=0) throw runtime_error("set_rfmon failed");
    if (pcap_set_timeout(ppcap, -1) !=0) throw runtime_error("set_timeout failed");
    //if (pcap_set_buffer_size(ppcap, 2048) !=0) throw runtime_error("set_buffer_size failed");
    if (pcap_activate(ppcap) !=0) throw runtime_error(string_format("pcap_activate failed: %s", pcap_geterr(ppcap)));
    if (pcap_setnonblock(ppcap, 1, errbuf) != 0) throw runtime_error(string_format("set_nonblock failed: %s", errbuf));

    int link_encap = pcap_datalink(ppcap);
    struct bpf_program bpfprogram;
    string program;

    switch (link_encap)
    {
    case DLT_PRISM_HEADER:
        fprintf(stderr, "%s has DLT_PRISM_HEADER Encap\n", wlan);
        program = string_format("radio[0x4a:4]==0x13223344 && radio[0x4e:2] == 0x55%.2x", radio_port);
        break;

    case DLT_IEEE802_11_RADIO:
        fprintf(stderr, "%s has DLT_IEEE802_11_RADIO Encap\n", wlan);
        program = string_format("ether[0x0a:4]==0x13223344 && ether[0x0e:2] == 0x55%.2x", radio_port);
        break;

    default:
        throw runtime_error(string_format("unknown encapsulation on %s", wlan));
    }

    if (pcap_compile(ppcap, &bpfprogram, program.c_str(), 1, 0) == -1) {
        throw runtime_error(string_format("Unable to compile %s: %s", program.c_str(), pcap_geterr(ppcap)));
    }

    if (pcap_setfilter(ppcap, &bpfprogram) == -1) {
        throw runtime_error(string_format("Unable to set filter %s: %s", program.c_str(), pcap_geterr(ppcap)));
    }

    pcap_freecode(&bpfprogram);
    fd = pcap_get_selectable_fd(ppcap);
}


Receiver::~Receiver()
{
    close(fd);
    pcap_close(ppcap);
}


void Receiver::loop_iter(void)
{
    for(;;) // loop while incoming queue is not empty
    {
        struct pcap_pkthdr hdr;
        const uint8_t* pkt = pcap_next(ppcap, &hdr);

        if (pkt == NULL) {
            break;
        }

        int pktlen = hdr.caplen;
        // int pkt_rate = 0
        int ant_idx = 0;
        uint8_t antenna[RX_ANT_MAX];
        int8_t rssi[RX_ANT_MAX];
        uint8_t flags = 0;
        struct ieee80211_radiotap_iterator iterator;
        int ret = ieee80211_radiotap_iterator_init(&iterator, (ieee80211_radiotap_header*)pkt, pktlen, NULL);

        // Fill all antenna slots with 0xff (unused)
        memset(antenna, 0xff, sizeof(antenna));
        // Fill all rssi slots with minimum value
        memset(rssi, SCHAR_MIN, sizeof(rssi));

        while (ret == 0 && ant_idx < RX_ANT_MAX) {
            ret = ieee80211_radiotap_iterator_next(&iterator);

            if (ret)
                continue;

            /* see if this argument is something we can use */

            switch (iterator.this_arg_index)
            {
                /*
                 * You must take care when dereferencing iterator.this_arg
                 * for multibyte types... the pointer is not aligned.  Use
                 * get_unaligned((type *)iterator.this_arg) to dereference
                 * iterator.this_arg for type "type" safely on all arches.
                 */

                // case IEEE80211_RADIOTAP_RATE:
                //     /* radiotap "rate" u8 is in
                //      * 500kbps units, eg, 0x02=1Mbps
                //      */
                //     pkt_rate = (*(uint8_t*)(iterator.this_arg))/2;
                //     break;

            case IEEE80211_RADIOTAP_ANTENNA:
                // FIXME
                // In case of multiple antenna stats in one packet this index will be irrelivant
                antenna[ant_idx] = *(uint8_t*)(iterator.this_arg);
                ant_idx += 1;
                break;

            case IEEE80211_RADIOTAP_DBM_ANTSIGNAL:
                // Some cards can provide rssi for multiple antennas in one packet, so we should select maximum value
                rssi[ant_idx] = *(int8_t*)(iterator.this_arg);
                break;

            case IEEE80211_RADIOTAP_FLAGS:
                flags = *(uint8_t*)(iterator.this_arg);
                break;

            default:
                break;
            }
        }  /* while more rt headers */

        if (ret != -ENOENT && ant_idx < RX_ANT_MAX){
            fprintf(stderr, "Error parsing radiotap header!\n");
            continue;
        }

        if (flags & IEEE80211_RADIOTAP_F_FCS)
        {
            pktlen -= 4;
        }

        if (flags & IEEE80211_RADIOTAP_F_BADFCS)
        {
            fprintf(stderr, "Got packet with bad fsc\n");
            continue;
        }

        /* discard the radiotap header part */
        pkt += iterator._max_length;
        pktlen -= iterator._max_length;

        if (pktlen > (int)sizeof(ieee80211_header))
        {
            agg->process_packet(pkt + sizeof(ieee80211_header), pktlen - sizeof(ieee80211_header), wlan_idx, antenna, rssi, NULL);
        } else {
            fprintf(stderr, "short packet (ieee header)\n");
            continue;
        }
    }
}


Aggregator::Aggregator(const string &client_addr, int client_port, int k, int n, const string &keypair) : fec_k(k), fec_n(n), seq(0), rx_ring_front(0), rx_ring_alloc(0), last_known_block((uint64_t)-1),
                                                                                                          count_p_all(0), count_p_dec_err(0), count_p_dec_ok(0), count_p_fec_recovered(0),
                                                                                                          count_p_lost(0), count_p_bad(0)
{
    sockfd = open_udp_socket_for_tx(client_addr, client_port);
    fec_p = fec_new(fec_k, fec_n);
    memset(session_key, '\0', sizeof(session_key));

    for(int ring_idx = 0; ring_idx < RX_RING_SIZE; ring_idx++)
    {
        rx_ring[ring_idx].block_idx = 0;
        rx_ring[ring_idx].send_fragment_idx = 0;
        rx_ring[ring_idx].has_fragments = 0;
        rx_ring[ring_idx].fragments = new uint8_t*[fec_n];
        for(int i=0; i < fec_n; i++)
        {
            rx_ring[ring_idx].fragments[i] = new uint8_t[MAX_FEC_PAYLOAD];
        }
        rx_ring[ring_idx].fragment_map = new uint8_t[fec_n];
        memset(rx_ring[ring_idx].fragment_map, '\0', fec_n * sizeof(uint8_t));
    }

    FILE *fp;
    if((fp = fopen(keypair.c_str(), "r")) == NULL)
    {
        throw runtime_error(string_format("Unable to open %s: %s", keypair.c_str(), strerror(errno)));
    }
    if (fread(rx_secretkey, crypto_box_SECRETKEYBYTES, 1, fp) != 1) throw runtime_error(string_format("Unable to read rx secret key: %s", strerror(errno)));
    if (fread(tx_publickey, crypto_box_PUBLICKEYBYTES, 1, fp) != 1) throw runtime_error(string_format("Unable to read tx public key: %s", strerror(errno)));
    fclose(fp);
}


Aggregator::~Aggregator()
{

    for(int ring_idx = 0; ring_idx < RX_RING_SIZE; ring_idx++)
    {
        delete rx_ring[ring_idx].fragment_map;
        for(int i=0; i < fec_n; i++)
        {
            delete rx_ring[ring_idx].fragments[i];
        }
        delete rx_ring[ring_idx].fragments;
    }
    close(sockfd);
}


Forwarder::Forwarder(const string &client_addr, int client_port)
{
    sockfd = open_udp_socket_for_tx(client_addr, client_port);
}


void Forwarder::process_packet(const uint8_t *buf, size_t size, uint8_t wlan_idx, const uint8_t *antenna, const int8_t *rssi, sockaddr_in *sockaddr)
{
    wrxfwd_t fwd_hdr = { .wlan_idx = wlan_idx };

    memcpy(fwd_hdr.antenna, antenna, RX_ANT_MAX * sizeof(uint8_t));
    memcpy(fwd_hdr.rssi, rssi, RX_ANT_MAX * sizeof(int8_t));

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


Forwarder::~Forwarder()
{
    close(sockfd);
}

int Aggregator::rx_ring_push(void)
{
    if(rx_ring_alloc < RX_RING_SIZE)
    {
        int idx = modN(rx_ring_front + rx_ring_alloc, RX_RING_SIZE);
        rx_ring_alloc += 1;
        return idx;
    }

    // override existing data
    int idx = rx_ring_front;

    /*
      Ring overflow. This means that there are more unfinished blocks than ring size
      Possible solutions:
      1. Increase ring size. Do this if you have large variance of packet travel time throught WiFi card or network stack.
         Some cards can do this due to packet reordering inside, diffent chipset and/or firmware or your RX hosts have different CPU power.
      2. Reduce packet injection speed or try to unify RX hardware.
    */

    fprintf(stderr, "override block 0x%" PRIx64 " with %d fragments\n", rx_ring[idx].block_idx, rx_ring[idx].has_fragments);

    rx_ring_front = modN(rx_ring_front + 1, RX_RING_SIZE);
    return idx;
}


int Aggregator::get_block_ring_idx(uint64_t block_idx)
{
    // check if block is already to the ring
    for(int i = rx_ring_front, c = rx_ring_alloc; c > 0; i = modN(i + 1, RX_RING_SIZE), c--)
    {
        if (rx_ring[i].block_idx == block_idx) return i;
    }

    // check if block is already known and not in the ring then it is already processed
    if (last_known_block != (uint64_t)-1 && block_idx <= last_known_block)
    {
        return -1;
    }

    int new_blocks = (int)min(last_known_block != (uint64_t)-1 ? block_idx - last_known_block : 1, (uint64_t)RX_RING_SIZE);
    assert (new_blocks > 0);

    last_known_block = block_idx;
    int ring_idx = -1;

    for(int i = 0; i < new_blocks; i++)
    {
        ring_idx = rx_ring_push();
        rx_ring[ring_idx].block_idx = block_idx + i + 1 - new_blocks;
        rx_ring[ring_idx].send_fragment_idx = 0;
        rx_ring[ring_idx].has_fragments = 0;
        memset(rx_ring[ring_idx].fragment_map, '\0', fec_n * sizeof(uint8_t));
    }
    return ring_idx;
}

void Aggregator::dump_stats(FILE *fp)
{
    //timestamp in ms
    uint64_t ts = get_time_ms();

    for(antenna_stat_t::iterator it = antenna_stat.begin(); it != antenna_stat.end(); it++)
    {
        fprintf(fp, "%" PRIu64 "\tANT\t%" PRIx64 "\t%d:%d:%d:%d\n", ts, it->first, it->second.count_all, it->second.rssi_min, it->second.rssi_sum / it->second.count_all, it->second.rssi_max);
    }
    antenna_stat.clear();

    fprintf(fp, "%" PRIu64 "\tPKT\t%u:%u:%u:%u:%u:%u\n", ts, count_p_all, count_p_dec_err, count_p_dec_ok, count_p_fec_recovered, count_p_lost, count_p_bad);
    fflush(fp);

    count_p_all = 0;
    count_p_dec_err = 0;
    count_p_dec_ok = 0;
    count_p_fec_recovered = 0;
    count_p_lost = 0;
    count_p_bad = 0;
}


void Aggregator::log_rssi(const sockaddr_in *sockaddr, uint8_t wlan_idx, const uint8_t *ant, const int8_t *rssi)
{
    for(int i = 0; i < RX_ANT_MAX && ant[i] != 0xff; i++)
    {
        // key: addr + port + wlan_idx + ant
        uint64_t key = 0;
        if (sockaddr != NULL && sockaddr->sin_family == AF_INET)
        {
            key = ((uint64_t)ntohl(sockaddr->sin_addr.s_addr) << 32 | (uint64_t)ntohs(sockaddr->sin_port) << 16);
        }

        key |= ((uint64_t)wlan_idx << 8 | (uint64_t)ant[i]);

        antenna_stat[key].log_rssi(rssi[i]);
    }
}


void Aggregator::process_packet(const uint8_t *buf, size_t size, uint8_t wlan_idx, const uint8_t *antenna, const int8_t *rssi, sockaddr_in *sockaddr)
{
    uint8_t new_session_key[sizeof(session_key)];
    count_p_all += 1;

    if(size == 0) return;

    if (size > MAX_FORWARDER_PACKET_SIZE)
    {
        fprintf(stderr, "long packet (fec payload)\n");
        count_p_bad += 1;
        return;
    }

    switch(buf[0])
    {
    case WFB_PACKET_DATA:
        if(size < sizeof(wblock_hdr_t) + sizeof(wpacket_hdr_t))
        {
            fprintf(stderr, "short packet (fec header)\n");
            count_p_bad += 1;
            return;
        }
        break;

    case WFB_PACKET_KEY:
        if(size != sizeof(wsession_key_t))
        {
            fprintf(stderr, "invalid session key packet\n");
            count_p_bad += 1;
            return;
        }

        if(crypto_box_open_easy(new_session_key,
                                ((wsession_key_t*)buf)->session_key_data, sizeof(wsession_key_t::session_key_data),
                                ((wsession_key_t*)buf)->session_key_nonce,
                                tx_publickey, rx_secretkey) != 0)
        {
            fprintf(stderr, "unable to decrypt session key\n");
            count_p_dec_err += 1;
            return;
        }

        count_p_dec_ok += 1;

        if (memcmp(session_key, new_session_key, sizeof(session_key)) != 0)
        {
            fprintf(stderr, "New session detected\n");
            memcpy(session_key, new_session_key, sizeof(session_key));

            rx_ring_front = 0;
            rx_ring_alloc = 0;
            last_known_block = (uint64_t)-1;
            seq = 0;
            for(int ring_idx = 0; ring_idx < RX_RING_SIZE; ring_idx++)
            {
                rx_ring[ring_idx].block_idx = 0;
                rx_ring[ring_idx].send_fragment_idx = 0;
                rx_ring[ring_idx].has_fragments = 0;
                memset(rx_ring[ring_idx].fragment_map, '\0', fec_n * sizeof(uint8_t));
            }
        }
        return;

    default:
        fprintf(stderr, "Unknown packet type 0x%x\n", buf[0]);
        count_p_bad += 1;
        return;
    }

    uint8_t decrypted[MAX_FEC_PAYLOAD];
    long long unsigned int decrypted_len;
    wblock_hdr_t *block_hdr = (wblock_hdr_t*)buf;

    if (crypto_aead_chacha20poly1305_decrypt(decrypted, &decrypted_len,
                                             NULL,
                                             buf + sizeof(wblock_hdr_t), size - sizeof(wblock_hdr_t),
                                             buf,
                                             sizeof(wblock_hdr_t),
                                             (uint8_t*)(&(block_hdr->nonce)), session_key) != 0)
    {
        fprintf(stderr, "unable to decrypt packet #0x%" PRIx64 "\n", be64toh(block_hdr->nonce));
        count_p_dec_err += 1;
        return;
    }

    count_p_dec_ok += 1;
    log_rssi(sockaddr, wlan_idx, antenna, rssi);

    assert(decrypted_len <= MAX_FEC_PAYLOAD);

    uint64_t block_idx = be64toh(block_hdr->nonce) >> 8;
    uint8_t fragment_idx = (uint8_t)(be64toh(block_hdr->nonce) & 0xff);

    // Should never happend due to generating new session key on tx side
    if (block_idx > MAX_BLOCK_IDX)
    {
        fprintf(stderr, "block_idx overflow\n");
        count_p_bad += 1;
        return;
    }

    if (fragment_idx >= fec_n)
    {
        fprintf(stderr, "invalid fragment_idx: %d\n", fragment_idx);
        count_p_bad += 1;
        return;
    }

    int ring_idx = get_block_ring_idx(block_idx);

    //printf("got 0x%lx %d, ring_idx=%d\n", block_idx, fragment_idx, ring_idx);

    //ignore already processed blocks
    if (ring_idx < 0) return;

    rx_ring_item_t *p = &rx_ring[ring_idx];

    //ignore already processed fragments
    if (p->fragment_map[fragment_idx]) return;

    memset(p->fragments[fragment_idx], '\0', MAX_FEC_PAYLOAD);
    memcpy(p->fragments[fragment_idx], decrypted, decrypted_len);

    p->fragment_map[fragment_idx] = 1;
    p->has_fragments += 1;

    if(ring_idx == rx_ring_front)
    {
        // check if any packets without gaps
        while(p->send_fragment_idx < fec_k && p->fragment_map[p->send_fragment_idx])
        {
            send_packet(ring_idx, p->send_fragment_idx);
            p->send_fragment_idx += 1;
        }
    }

    // or we can reconstruct gaps via FEC
    if(p->send_fragment_idx < fec_k && p->has_fragments == fec_k)
    {
        //printf("do fec\n");
        apply_fec(ring_idx);
        while(p->send_fragment_idx < fec_k)
        {
            count_p_fec_recovered += 1;
            send_packet(ring_idx, p->send_fragment_idx);
            p->send_fragment_idx += 1;
        }
    }

    if(p->send_fragment_idx == fec_k)
    {
        int nrm = modN(ring_idx - rx_ring_front, RX_RING_SIZE);
        for(int i=0; i <= nrm; i++)
        {
            rx_ring_front = modN(rx_ring_front + 1, RX_RING_SIZE);
            rx_ring_alloc -= 1;
        }
        assert(rx_ring_alloc >= 0);
    }
}

void Aggregator::send_packet(int ring_idx, int fragment_idx)
{
    wpacket_hdr_t* packet_hdr = (wpacket_hdr_t*)(rx_ring[ring_idx].fragments[fragment_idx]);
    uint8_t *payload = (rx_ring[ring_idx].fragments[fragment_idx]) + sizeof(wpacket_hdr_t);
    uint16_t packet_size = be16toh(packet_hdr->packet_size);
    uint32_t packet_seq = rx_ring[ring_idx].block_idx * fec_k + fragment_idx;

    if (packet_seq > seq + 1)
    {
        fprintf(stderr, "%u packets lost\n", packet_seq - seq - 1);
        count_p_lost += (packet_seq - seq - 1);
    }

    seq = packet_seq;

    if(packet_size > MAX_PAYLOAD_SIZE)
    {
        fprintf(stderr, "corrupted packet %u\n", seq);
        count_p_bad += 1;
    }else{
        send(sockfd, payload, packet_size, MSG_DONTWAIT);
    }
}

void Aggregator::apply_fec(int ring_idx)
{
    unsigned index[fec_k];
    uint8_t *in_blocks[fec_k];
    uint8_t *out_blocks[fec_n - fec_k];
    int j = fec_k;
    int ob_idx = 0;

    for(int i=0; i < fec_k; i++)
    {
        if(rx_ring[ring_idx].fragment_map[i])
        {
            in_blocks[i] = rx_ring[ring_idx].fragments[i];
            index[i] = i;
        }else
        {
            for(;j < fec_n; j++)
            {
                if(rx_ring[ring_idx].fragment_map[j])
                {
                    in_blocks[i] = rx_ring[ring_idx].fragments[j];
                    out_blocks[ob_idx++] = rx_ring[ring_idx].fragments[i];
                    index[i] = j;
                    j++;
                    break;
                }
            }
        }
    }
    fec_decode(fec_p, (const uint8_t**)in_blocks, out_blocks, index, MAX_FEC_PAYLOAD);
}

void radio_loop(int argc, char* const *argv, int optind, int radio_port, shared_ptr<BaseAggregator> agg, int log_interval)
{
    int nfds = min(argc - optind, MAX_RX_INTERFACES);
    uint64_t log_send_ts = 0;
    struct pollfd fds[MAX_RX_INTERFACES];
    Receiver* rx[MAX_RX_INTERFACES];

    memset(fds, '\0', sizeof(fds));

    for(int i = 0; i < nfds; i++)
    {
        rx[i] = new Receiver(argv[optind + i], i, radio_port, agg.get());
        fds[i].fd = rx[i]->getfd();
        fds[i].events = POLLIN;
    }

    for(;;)
    {
        uint64_t cur_ts = get_time_ms();
        int rc = poll(fds, nfds, log_send_ts > cur_ts ? log_send_ts - cur_ts : 0);

        if (rc < 0){
            if (errno == EINTR || errno == EAGAIN) continue;
            throw runtime_error(string_format("Poll error: %s", strerror(errno)));
        }

        cur_ts = get_time_ms();

        if (cur_ts >= log_send_ts)
        {
            agg->dump_stats(stdout);
            log_send_ts = get_time_ms() + log_interval;
        }

        if (rc == 0) continue; // timeout expired

        for(int i = 0; rc > 0 && i < nfds; i++)
        {
            if (fds[i].revents & (POLLERR|POLLNVAL))
            {
                throw runtime_error("socket error!");
            }
            if (fds[i].revents & POLLIN){
                rx[i]->loop_iter();
                rc -= 1;
            }
        }
    }
}

void network_loop(int srv_port, Aggregator &agg, int log_interval)
{
    wrxfwd_t fwd_hdr;
    struct sockaddr_in sockaddr;
    uint8_t buf[MAX_FORWARDER_PACKET_SIZE];

    uint64_t log_send_ts = 0;
    struct pollfd fds[1];
    int fd = open_udp_socket_for_rx(srv_port);

    if(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) < 0)
    {
        throw runtime_error(string_format("Unable to set socket into nonblocked mode: %s", strerror(errno)));
    }

    memset(fds, '\0', sizeof(fds));
    fds[0].fd = fd;
    fds[0].events = POLLIN;

    for(;;)
    {
        uint64_t cur_ts = get_time_ms();
        int rc = poll(fds, 1, log_send_ts > cur_ts ? log_send_ts - cur_ts : 0);

        if (rc < 0){
            if (errno == EINTR || errno == EAGAIN) continue;
            throw runtime_error(string_format("poll error: %s", strerror(errno)));
        }

        cur_ts = get_time_ms();

        if (cur_ts >= log_send_ts)
        {
            agg.dump_stats(stdout);
            log_send_ts = get_time_ms() + log_interval;
        }

        if (rc == 0) continue; // timeout expired

        // some events detected
        if (fds[0].revents & (POLLERR | POLLNVAL))
        {
            throw runtime_error(string_format("socket error: %s", strerror(errno)));
        }

        if (fds[0].revents & POLLIN)
        {
            for(;;) // process pending rx
            {
                memset((void*)&sockaddr, '\0', sizeof(sockaddr));

                struct iovec iov[2] = {{ .iov_base = (void*)&fwd_hdr,
                                         .iov_len = sizeof(fwd_hdr)},
                                       { .iov_base = (void*)buf,
                                         .iov_len = sizeof(buf) }};

                struct msghdr msghdr = { .msg_name = (void*)&sockaddr,
                                         .msg_namelen = sizeof(sockaddr),
                                         .msg_iov = iov,
                                         .msg_iovlen = 2,
                                         .msg_control = NULL,
                                         .msg_controllen = 0,
                                         .msg_flags = 0};

                ssize_t rsize = recvmsg(fd, &msghdr, 0);
                if (rsize < 0)
                {
                    break;
                }

                if (rsize < (ssize_t)sizeof(wrxfwd_t))
                {
                    fprintf(stderr, "short packet (rx fwd header)\n");
                    continue;
                }
                agg.process_packet(buf, rsize - sizeof(wrxfwd_t), fwd_hdr.wlan_idx, fwd_hdr.antenna, fwd_hdr.rssi, &sockaddr);
            }
            if(errno != EWOULDBLOCK) throw runtime_error(string_format("Error receiving packet: %s", strerror(errno)));
        }
    }
}

int main(int argc, char* const *argv)
{
    int opt;
    uint8_t k = 8, n = 12, radio_port = 1;
    int log_interval = 1000;
    int client_port = 5600;
    int srv_port = 0;
    string client_addr = "127.0.0.1";
    rx_mode_t rx_mode = LOCAL;
    string keypair = "rx.key";

    while ((opt = getopt(argc, argv, "K:fa:k:n:c:u:p:l:")) != -1) {
        switch (opt) {
        case 'K':
            keypair = optarg;
            break;
        case 'f':
            rx_mode = FORWARDER;
            break;
        case 'a':
            rx_mode = AGGREGATOR;
            srv_port = atoi(optarg);
            break;
        case 'k':
            k = atoi(optarg);
            break;
        case 'n':
            n = atoi(optarg);
            break;
        case 'c':
            client_addr = string(optarg);
            break;
        case 'u':
            client_port = atoi(optarg);
            break;
        case 'p':
            radio_port = atoi(optarg);
            break;
        case 'l':
            log_interval = atoi(optarg);
            break;
        default: /* '?' */
        show_usage:
            fprintf(stderr, "Local receiver: %s [-K rx_key] [-k RS_K] [-n RS_N] [-c client_addr] [-u client_port] [-p radio_port] [-l log_interval] interface1 [interface2] ...\n", argv[0]);
            fprintf(stderr, "Remote (forwarder): %s -f [-c client_addr] [-u client_port] [-p radio_port] interface1 [interface2] ...\n", argv[0]);
            fprintf(stderr, "Remote (aggregator): %s -a server_port [-K rx_key] [-k RS_K] [-n RS_N] [-c client_addr] [-u client_port] [-l log_interval]\n", argv[0]);
            fprintf(stderr, "Default: K='%s', k=%d, n=%d, connect=%s:%d, radio_port=%d, log_interval=%d\n", keypair.c_str(), k, n, client_addr.c_str(), client_port, radio_port, log_interval);
            fprintf(stderr, "WFB version " WFB_VERSION "\n");
            exit(1);
        }
    }

    try
    {
        if (rx_mode == LOCAL || rx_mode == FORWARDER)
        {
            if (optind >= argc) goto show_usage;

            shared_ptr<BaseAggregator> agg;
            if(rx_mode == LOCAL){
                agg = shared_ptr<Aggregator>(new Aggregator(client_addr, client_port, k, n, keypair));
            }else{
                agg = shared_ptr<Forwarder>(new Forwarder(client_addr, client_port));
            }

            radio_loop(argc, argv, optind, radio_port, agg, log_interval);
        }else if(rx_mode == AGGREGATOR)
        {
            if (optind > argc) goto show_usage;
            Aggregator agg(client_addr, client_port, k, n, keypair);

            network_loop(srv_port, agg, log_interval);
        }else{
            throw runtime_error(string_format("Unknown rx_mode=%d", rx_mode));
        }
    }catch(runtime_error &e)
    {
        fprintf(stderr, "Error: %s\n", e.what());
        exit(1);
    }
    return 0;
}
