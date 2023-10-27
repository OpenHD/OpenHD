//
// Created by consti10 on 13.09.23.
//

#ifndef OPENHD_WBLINKMANAGER_H
#define OPENHD_WBLINKMANAGER_H

#include <utility>
#include <variant>
#include <optional>
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include "../../lib/wifibroadcast/src/WBTxRx.h"

/**
 * Quite a lot of complicated code to implement 40Mhz without sync of air and ground
 * worth it, though ;)
 * We have public std::atomic members, since the data only needs to be accessed/written atomically from the wb_link
 * worker thread.
 */

class ManagementAir{
public:
    explicit ManagementAir(std::shared_ptr<WBTxRx> wb_tx_rx,int initial_freq_mhz,int inital_channel_width_mhz);
    ManagementAir(const ManagementAir&)=delete;
    ManagementAir(const ManagementAir&&)=delete;
    ~ManagementAir();
    void start();
    // TODO dirty
    std::shared_ptr<RadiotapHeaderTxHolder> m_tx_header;
public:
    std::atomic<uint32_t> m_curr_frequency_mhz;
    std::atomic<uint8_t> m_curr_channel_width_mhz;
    std::atomic<int> m_last_channel_width_change_timestamp_ms;
    int get_last_received_packet_ts_ms();
private:
    void loop();
    void on_new_management_packet(const uint8_t *data, int data_len);
    std::shared_ptr<WBTxRx> m_wb_txrx;
    std::shared_ptr<spdlog::logger> m_console;
    std::atomic<bool> m_tx_thread_run= true;
    std::unique_ptr<std::thread> m_tx_thread;
    std::chrono::steady_clock::time_point m_air_last_management_frame=std::chrono::steady_clock::now();
    std::atomic<int> m_last_received_packet_timestamp_ms=0;
};

class ManagementGround{
public:
    explicit ManagementGround(std::shared_ptr<WBTxRx> wb_tx_rx);
    void start();
    ManagementGround(const ManagementGround&)=delete;
    ManagementGround(const ManagementGround&&)=delete;
    ~ManagementGround();
    // TODO dirty
    std::shared_ptr<RadiotapHeaderTxHolder> m_tx_header;
public:
    std::atomic<int> m_air_reported_curr_frequency=-1;
    std::atomic<int> m_air_reported_curr_channel_width=-1;
    int get_last_received_packet_ts_ms();
private:
    void loop();
    std::shared_ptr<WBTxRx> m_wb_txrx;
    std::shared_ptr<spdlog::logger> m_console;
    std::atomic<bool> m_tx_thread_run= true;
    std::unique_ptr<std::thread> m_tx_thread;
    std::atomic<int> m_last_received_packet_timestamp_ms=0;
    // 40Mhz / 20Mhz link management
    void on_new_management_packet(const uint8_t *data, int data_len);
};


#endif //OPENHD_WBLINKMANAGER_H
