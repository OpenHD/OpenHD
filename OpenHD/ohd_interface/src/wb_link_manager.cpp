//
// Created by consti10 on 13.09.23.
//

#include <cstring>
#include <sstream>
#include "wb_link_manager.h"
#include "openhd_spdlog.h"
#include "openhd_util.h"

static constexpr auto MANAGEMENT_RADIO_PORT_AIR_TX=20;

namespace openhd::wb {

static constexpr uint8_t MNGMNT_PACKET_ID_CHANNEL_WIDTH=0;

struct ManagementFrameData{
    uint32_t center_frequency_mhz;
    uint8_t bandwidth_mhz;
}__attribute__ ((packed));
static std::optional<ManagementFrameData> parse_data_management(const uint8_t *data, const int data_len) {
    if(data_len==sizeof(ManagementFrameData)+1 && data[0]==MNGMNT_PACKET_ID_CHANNEL_WIDTH){
        ManagementFrameData ret{};
        std::memcpy(&ret,&data[1],data_len);
        return ret;
    }
    return std::nullopt;
}

static std::vector<uint8_t> pack_management_frame(const ManagementFrameData& data) {
    std::vector<uint8_t> ret;
    ret.resize(1+sizeof(ManagementFrameData));
    ret[0]=MNGMNT_PACKET_ID_CHANNEL_WIDTH;
    std::memcpy(&ret[1],(void*)&data,sizeof(ManagementFrameData));
    return ret;
}

static std::string management_frame_to_string(const ManagementFrameData &data) {
    return fmt::format("Center: {}Mhz BW:{}Mhz",(int)data.center_frequency_mhz,(int)data.bandwidth_mhz);
}

}

ManagementAir::ManagementAir(std::shared_ptr<WBTxRx> wb_tx_rx) :m_wb_txrx(std::move(wb_tx_rx)){
    m_console = openhd::log::create_or_get("wb_mngmt_air");
    m_last_channel_width_change_timestamp_ms=OHDUtil::steady_clock_time_epoch_ms();
}

void ManagementAir::start() {
    m_tx_thread_run= true;
    m_tx_thread=std::make_unique<std::thread>(&ManagementAir::loop, this);
}

ManagementAir::~ManagementAir() {
    m_tx_thread_run= false;
    m_tx_thread->join();
    m_tx_thread= nullptr;
}

void ManagementAir::loop() {
    while (m_tx_thread_run){
        // Air: Continuously broadcast channel width
        // Calculate the interval in which we broadcast the channel width management frame
        std::chrono::duration management_frame_interval=std::chrono::milliseconds(500); // default 2Hz
        const auto elapsed_since_last_change=OHDUtil::steady_clock_time_epoch_ms()-m_last_channel_width_change_timestamp_ms;
        if(elapsed_since_last_change<5*1000){
            // If the last change is recent, send in higher interval (10Hz)
            management_frame_interval=std::chrono::milliseconds(100);
        }
        const auto elapsed_since_last_management_frame=std::chrono::steady_clock::now()-m_air_last_management_frame;
        if(elapsed_since_last_management_frame<management_frame_interval){
            return;
        }
        openhd::wb::ManagementFrameData managementFrame{m_curr_frequency_mhz.load(),m_curr_channel_width_mhz.load()};
        auto data=openhd::wb::pack_management_frame(managementFrame);
        auto radiotap_header=m_tx_header_2->thread_safe_get();
        m_wb_txrx->tx_inject_packet(MANAGEMENT_RADIO_PORT_AIR_TX,data.data(),data.size(),radiotap_header,true);
        m_console->debug("Sent mngmt");
        std::this_thread::sleep_for(std::chrono::milliseconds(management_frame_interval));
    }
}

ManagementGround::ManagementGround(std::shared_ptr<WBTxRx> wb_tx_rx) :m_wb_txrx(std::move(wb_tx_rx)){
    m_console = openhd::log::create_or_get("wb_mngmt_gnd");
    auto cb_packet=[this](uint64_t nonce,int wlan_index,const uint8_t *data, const int data_len){
        this->on_new_management_packet(data,data_len);
    };
    auto mgmt_handler=std::make_shared<WBTxRx::StreamRxHandler>(MANAGEMENT_RADIO_PORT_AIR_TX,cb_packet, nullptr);
    m_wb_txrx->rx_register_stream_handler(mgmt_handler);
}

void ManagementGround::on_new_management_packet(const uint8_t *data, int data_len) {
    const auto opt_mngmt=openhd::wb::parse_data_management(data,data_len);
    if(opt_mngmt.has_value()){
        const auto packet=opt_mngmt.value();
        m_console->debug("Got MNGMT {}",openhd::wb::management_frame_to_string(packet));
        if(packet.bandwidth_mhz==20 || packet.bandwidth_mhz==40){
            m_air_reported_curr_channel_width=packet.bandwidth_mhz;
            m_air_reported_curr_frequency=packet.center_frequency_mhz;
        }else{
            m_console->warn("Air reports invalid bandwidth {}",packet.bandwidth_mhz);
        }
    }
}

ManagementGround::~ManagementGround() {
    m_wb_txrx->rx_unregister_stream_handler(MANAGEMENT_RADIO_PORT_AIR_TX);
}

