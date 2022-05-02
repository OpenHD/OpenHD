//
// Created by consti10 on 12.12.20.
//

#ifndef WIFIBROADCAST_RAWTRANSMITTER_HPP
#define WIFIBROADCAST_RAWTRANSMITTER_HPP

#include "Ieee80211Header.hpp"
#include "RadiotapHeader.hpp"

#include <cstdlib>
#include <endian.h>
#include <fcntl.h>
#include <ctime>
#include <sys/mman.h>
#include <string>
#include <vector>
#include <chrono>
#include <optional>
#include <poll.h>
#include <pcap.h>

// This is a single header-only file you can use to build your own wifibroadcast link
// It doesn't specify if / what FEC to use and so on

// Doesn't specify what / how big the custom header is.
// This way it is easy to make the injection part generic for future changes
// by using a pointer / size tuple the data for the customHeader and payload can reside at different memory locations
// When injecting the packet we have to always copy the data anyways since Radiotap and IEE80211 header
// are stored at different locations, too
class AbstractWBPacket{
public:
    // constructor for packet without header (or the header is already merged into payload)
    AbstractWBPacket(const uint8_t *payload,const std::size_t payloadSize):
            customHeader(nullptr),customHeaderSize(0),payload(payload),payloadSize(payloadSize){};
    // constructor for packet with header and payload at different memory locations
    AbstractWBPacket(const uint8_t *customHeader,const std::size_t customHeaderSize,const uint8_t *payload,const std::size_t payloadSize):
            customHeader(customHeader),customHeaderSize(customHeaderSize),payload(payload),payloadSize(payloadSize){};
    AbstractWBPacket(AbstractWBPacket&)=delete;
    AbstractWBPacket(AbstractWBPacket&&)=delete;
public:
    // can be nullptr if size 0
    const uint8_t *customHeader;
    // can be 0 for special use cases
    const std::size_t customHeaderSize;
    // can be nullptr if size 0
    const uint8_t *payload;
    // can be 0 for special use cases
    const std::size_t payloadSize;
};

namespace RawTransmitterHelper {
    // construct a radiotap packet with the following data layout:
    // [RadiotapHeader | Ieee80211Header | customHeader (if not size 0) | payload (if not size 0)]
    static std::vector<uint8_t>
    createRadiotapPacket(const RadiotapHeader &radiotapHeader, const Ieee80211Header &ieee80211Header,const AbstractWBPacket& abstractWbPacket) {
        const auto customHeaderAndPayloadSize=abstractWbPacket.customHeaderSize + abstractWbPacket.payloadSize;
        std::vector<uint8_t> packet(radiotapHeader.getSize() + ieee80211Header.getSize() + customHeaderAndPayloadSize);
        uint8_t *p = packet.data();
        // radiotap wbDataHeader
        memcpy(p, radiotapHeader.getData(), radiotapHeader.getSize());
        p += radiotapHeader.getSize();
        // ieee80211 wbDataHeader
        memcpy(p, ieee80211Header.getData(), ieee80211Header.getSize());
        p += ieee80211Header.getSize();
        if(abstractWbPacket.customHeaderSize>0){
            // customHeader
            memcpy(p, abstractWbPacket.customHeader,abstractWbPacket.customHeaderSize);
            p+=abstractWbPacket.customHeaderSize;
        }
        if(abstractWbPacket.payloadSize>0){
            // payload
            memcpy(p,abstractWbPacket.payload,abstractWbPacket.payloadSize);
        }
        return packet;
    }
    // throw runtime exception if injecting pcap packet goes wrong (should never happen)
    static void injectPacket(pcap_t *pcap, const std::vector<uint8_t> &packetData) {
        if (pcap_inject(pcap, packetData.data(), packetData.size()) != (int)packetData.size()) {
            throw std::runtime_error(StringFormat::convert("Unable to inject packet %s",pcap_geterr(pcap)));
        }
    }
    // copy paste from svpcom
    static pcap_t *openTxWithPcap(const std::string &wlan) {
        char errbuf[PCAP_ERRBUF_SIZE];
        pcap_t *p = pcap_create(wlan.c_str(), errbuf);
        if (p == nullptr) {
            throw std::runtime_error(StringFormat::convert("Unable to open interface %s in pcap: %s", wlan.c_str(), errbuf));
        }
        if (pcap_set_snaplen(p, 4096) != 0) throw std::runtime_error("set_snaplen failed");
        if (pcap_set_promisc(p, 1) != 0) throw std::runtime_error("set_promisc failed");
        //if (pcap_set_rfmon(p, 1) !=0) throw runtime_error("set_rfmon failed");
        if (pcap_set_timeout(p, -1) != 0) throw std::runtime_error("set_timeout failed");
        //if (pcap_set_buffer_size(p, 2048) !=0) throw runtime_error("set_buffer_size failed");
        // hm don't think it is needed on tx
        //better not enable stuff that is not needed if(pcap_set_immediate_mode(p,true)!=0)throw std::runtime_error(StringFormat::convert("pcap_set_immediate_mode failed: %s", pcap_geterr(p)));
        if (pcap_activate(p) != 0) throw std::runtime_error(StringFormat::convert("pcap_activate failed: %s", pcap_geterr(p)));
        //if (pcap_setnonblock(p, 1, errbuf) != 0) throw runtime_error(string_format("set_nonblock failed: %s", errbuf));
        return p;
    }
}

class IRawPacketInjector{
public:
    /**
     * Inject the packet data after prefixing it with Radiotap and IEEE80211 header
     * @return time it took to inject the packet
     */
    virtual std::chrono::steady_clock::duration injectPacket(const RadiotapHeader& radiotapHeader, const Ieee80211Header& ieee80211Header,const AbstractWBPacket& abstractWbPacket)const=0;
};

// Pcap Transmitter injects packets into the wifi adapter using pcap
// It does not specify what the payload is and therefore is just a really small wrapper around the pcap interface
// that properly opens / closes the interface on construction/destruction
class PcapTransmitter:public IRawPacketInjector{
public:
    explicit PcapTransmitter(const std::string &wlan){
        ppcap=RawTransmitterHelper::openTxWithPcap(wlan);
    }
    ~PcapTransmitter(){
        pcap_close(ppcap);
    }
    // inject packet by prefixing wifibroadcast packet with the IEE and Radiotap header
    // return: time it took to inject the packet.If the injection time is absurdly high, you might want to do something about it
    std::chrono::steady_clock::duration injectPacket(const RadiotapHeader& radiotapHeader, const Ieee80211Header& ieee80211Header,const AbstractWBPacket& abstractWbPacket)const{
        const auto packet = RawTransmitterHelper::createRadiotapPacket(radiotapHeader, ieee80211Header, abstractWbPacket);
        const auto before=std::chrono::steady_clock::now();
        RawTransmitterHelper::injectPacket(ppcap, packet);
        return std::chrono::steady_clock::now()-before;
    }
    void injectControllFrame(const RadiotapHeader& radiotapHeader,const std::vector<uint8_t>& iee80211ControllHeader){
        std::vector<uint8_t> packet(radiotapHeader.getSize()+iee80211ControllHeader.size());
        memcpy(packet.data(),&radiotapHeader,RadiotapHeader::SIZE_BYTES);
        memcpy(&packet[RadiotapHeader::SIZE_BYTES],iee80211ControllHeader.data(),iee80211ControllHeader.size());
        RawTransmitterHelper::injectPacket(ppcap, packet);
    }
private:
    pcap_t* ppcap;
};

// Doesn't use pcap but somehow directly talks to the OS via socket
// note that you still have to prefix data with the proper RadiotapHeader in this mode (just as if you were using pcap)
// NOTE: I didn't measure any advantage for RawSocketTransmitter compared to PcapTransmitter, so I'd recommend using PcapTransmitter only
class RawSocketTransmitter : public IRawPacketInjector{
public:
    explicit RawSocketTransmitter(const std::string &wlan) {
        sockFd= openWifiInterfaceAsTxRawSocket(wlan);
    }
    ~RawSocketTransmitter(){
        close(sockFd);
    }
    // inject packet by prefixing wifibroadcast packet with the IEE and Radiotap header
    // return: time it took to inject the packet.If the injection time is absurdly high, you might want to do something about it
    std::chrono::steady_clock::duration injectPacket(const RadiotapHeader& radiotapHeader, const Ieee80211Header& ieee80211Header,const AbstractWBPacket& abstractWbPacket)const{
        const auto packet = RawTransmitterHelper::createRadiotapPacket(radiotapHeader, ieee80211Header, abstractWbPacket);
        const auto before=std::chrono::steady_clock::now();
        if (write(sockFd,packet.data(),packet.size()) !=packet.size()) {
            throw std::runtime_error(StringFormat::convert("Unable to inject packet (raw sock) %s",strerror(errno)));
        }
        return std::chrono::steady_clock::now()-before;
    }
    // taken from https://github.com/OpenHD/Open.HD/blob/2.0/wifibroadcast-base/tx_rawsock.c#L86
    // open wifi interface using a socket (somehow this works ?!)
    static int openWifiInterfaceAsTxRawSocket(const std::string& wifi) {
        struct sockaddr_ll ll_addr{};
        struct ifreq ifr{};
        int sock = socket(AF_PACKET, SOCK_RAW, 0);
        if (sock == -1) {
            throw std::runtime_error(StringFormat::convert("Socket failed %s %s",wifi.c_str(),strerror(errno)));
        }

        ll_addr.sll_family = AF_PACKET;
        ll_addr.sll_protocol = 0;
        ll_addr.sll_halen = ETH_ALEN;

        strncpy(ifr.ifr_name, wifi.c_str(), IFNAMSIZ);

        if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
            throw std::runtime_error(StringFormat::convert("ioctl(SIOCGIFINDEX) failed\n"));
        }

        ll_addr.sll_ifindex = ifr.ifr_ifindex;

        if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
            throw std::runtime_error(StringFormat::convert("ioctl(SIOCGIFHWADDR) failed\n"));
        }

        memcpy(ll_addr.sll_addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);

        if (bind(sock, (struct sockaddr *)&ll_addr, sizeof(ll_addr)) == -1) {
            close(sock);
            throw std::runtime_error("bind failed\n");
        }
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 8000;
        if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
            throw std::runtime_error("setsockopt SO_SNDTIMEO\n");
        }
        int sendbuff = 131072;
        if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sendbuff, sizeof(sendbuff)) < 0) {
            throw std::runtime_error("setsockopt SO_SNDBUF\n");
        }
        return sock;
    }
private:
    int sockFd;
};

#endif //WIFIBROADCAST_RAWTRANSMITTER_HPP
