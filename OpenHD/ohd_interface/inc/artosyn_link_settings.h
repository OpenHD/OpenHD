#ifndef OPENHD_ARTOSYN_LINK_SETTINGS_H
#define OPENHD_ARTOSYN_LINK_SETTINGS_H

#include <string>

#include "openhd_settings_directories.h"
#include "openhd_settings_persistent.h"

namespace openhd {

// MAVLink parameter IDs must be <= 16 chars.
static constexpr auto AR_ADDR = "AR_ADDR";
static constexpr auto AR_PORT = "AR_PORT";
static constexpr auto AR_SLOT = "AR_SLOT";
static constexpr auto AR_VPORT = "AR_VPORT";
static constexpr auto AR_TPORT = "AR_TPORT";
static constexpr auto AR_DGRAM = "AR_DGRAM";
static constexpr auto AR_RXBUF = "AR_RXBUF";
static constexpr auto AR_TXBUF = "AR_TXBUF";
static constexpr auto AR_RDTMO = "AR_RDTMO";
static constexpr auto AR_DMN_AUTO = "AR_DMN_AUTO";
static constexpr auto AR_DMN_CMD = "AR_DMN_CMD";
static constexpr auto AR_MCS_MD = "AR_MCS_MD";
static constexpr auto AR_MCS_VAL = "AR_MCS_VAL";
static constexpr auto AR_MCS_MIN = "AR_MCS_MIN";
static constexpr auto AR_MCS_MAX = "AR_MCS_MAX";
static constexpr auto AR_BW_MD = "AR_BW_MD";
static constexpr auto AR_BW_VAL = "AR_BW_VAL";
static constexpr auto AR_CHN_MD = "AR_CHN_MD";
static constexpr auto AR_CHN_IDX = "AR_CHN_IDX";
static constexpr auto AR_PWR_ATO = "AR_PWR_ATO";
static constexpr auto AR_PWR_DBM = "AR_PWR_DBM";
static constexpr auto AR_BND_MD = "AR_BND_MD";
static constexpr auto AR_BND_VAL = "AR_BND_VAL";
static constexpr auto AR_CMP_MD = "AR_CMP_MD";
static constexpr auto AR_PWR_MD = "AR_PWR_MD";
static constexpr auto AR_LNA_MD = "AR_LNA_MD";
static constexpr auto AR_LNA_BP = "AR_LNA_BP";
static constexpr auto AR_RF_ATX = "AR_RF_ATX";
static constexpr auto AR_RF_ARX = "AR_RF_ARX";
static constexpr auto AR_RF_BTX = "AR_RF_BTX";
static constexpr auto AR_RF_BRX = "AR_RF_BRX";

// Read-only status
static constexpr auto AR_LK_STATE = "AR_LK_STATE";
static constexpr auto AR_RX_MCS = "AR_RX_MCS";
static constexpr auto AR_TX_MCS = "AR_TX_MCS";
static constexpr auto AR_BW = "AR_BW";
static constexpr auto AR_RX_BW = "AR_RX_BW";
static constexpr auto AR_PHY_TP = "AR_PHY_TP";
static constexpr auto AR_REAL_TP = "AR_REAL_TP";
static constexpr auto AR_TX_FREQ = "AR_TX_FREQ";
static constexpr auto AR_RX_FREQ = "AR_RX_FREQ";
static constexpr auto AR_SNR = "AR_SNR";
static constexpr auto AR_LDPC_E = "AR_LDPC_E";
static constexpr auto AR_LDPC_N = "AR_LDPC_N";
static constexpr auto AR_GAIN_A = "AR_GAIN_A";
static constexpr auto AR_GAIN_B = "AR_GAIN_B";
static constexpr auto AR_CHN_AUT = "AR_CHN_AUT";
static constexpr auto AR_CHN_CUR = "AR_CHN_CUR";
static constexpr auto AR_CHN_FK = "AR_CHN_FK";
static constexpr auto AR_PWR_CUR = "AR_PWR_CUR";
static constexpr auto AR_BND_AUT = "AR_BND_AUT";
static constexpr auto AR_BND_CUR = "AR_BND_CUR";
static constexpr auto AR_RF_ATX_R = "AR_RF_ATX_R";
static constexpr auto AR_RF_ARX_R = "AR_RF_ARX_R";
static constexpr auto AR_RF_BTX_R = "AR_RF_BTX_R";
static constexpr auto AR_RF_BRX_R = "AR_RF_BRX_R";
static constexpr auto AR_SW_VER = "AR_SW_VER";
static constexpr auto AR_HW_VER = "AR_HW_VER";
static constexpr auto AR_FW_VER = "AR_FW_VER";
static constexpr auto AR_CMP_TM = "AR_CMP_TM";
static constexpr auto AR_UPTIME = "AR_UPTIME";
static constexpr auto AR_RUNSYS = "AR_RUNSYS";
static constexpr auto AR_AP_TIME = "AR_AP_TIME";
static constexpr auto AR_TX_TPTH = "AR_TX_TPTH";
static constexpr auto AR_RX_TPTH = "AR_RX_TPTH";
static constexpr auto AR_P_SNR = "AR_P_SNR";
static constexpr auto AR_P_LDPC_E = "AR_P_LDPC_E";
static constexpr auto AR_P_LDPC_N = "AR_P_LDPC_N";
static constexpr auto AR_P_GAIN_A = "AR_P_GAIN_A";
static constexpr auto AR_P_GAIN_B = "AR_P_GAIN_B";
static constexpr auto AR_ROLE = "AR_ROLE";
static constexpr auto AR_MODE = "AR_MODE";
static constexpr auto AR_SYNC = "AR_SYNC";
static constexpr auto AR_SYNC_M = "AR_SYNC_M";
static constexpr auto AR_CFG_SBM = "AR_CFG_SBM";
static constexpr auto AR_RT_SBM = "AR_RT_SBM";
static constexpr auto AR_LMAC = "AR_LMAC";
static constexpr auto AR_PAIR = "AR_PAIR";
static constexpr auto AR_PMAC = "AR_PMAC";
static constexpr auto AR_TX_RFM = "AR_TX_RFM";
static constexpr auto AR_RX_RFM = "AR_RX_RFM";
static constexpr auto AR_TX_TEN = "AR_TX_TEN";
static constexpr auto AR_RX_TEN = "AR_RX_TEN";
static constexpr auto AR_TX_TNM = "AR_TX_TNM";
static constexpr auto AR_RX_TNM = "AR_RX_TNM";
static constexpr auto AR_TX_TLN = "AR_TX_TLN";
static constexpr auto AR_RX_TLN = "AR_RX_TLN";
static constexpr auto AR1_S_SNR = "AR1_S_SNR";
static constexpr auto AR1_S_LDPT = "AR1_S_LDPT";
static constexpr auto AR1_S_LDPN = "AR1_S_LDPN";
static constexpr auto AR1_S_GNA = "AR1_S_GNA";
static constexpr auto AR1_S_GNB = "AR1_S_GNB";
static constexpr auto AR1_S_MCS = "AR1_S_MCS";
static constexpr auto AR1_S_CHN = "AR1_S_CHN";
static constexpr auto AR1_S_PWR = "AR1_S_PWR";
static constexpr auto AR1_S_LNI = "AR1_S_LNI";
static constexpr auto AR1_S_LNF = "AR1_S_LNF";
static constexpr auto AR1_S_1TX = "AR1_S_1TX";
static constexpr auto AR1_S_TFK = "AR1_S_TFK";
static constexpr auto AR1_S_LSN = "AR1_S_LSN";
static constexpr auto AR1_S_LGA = "AR1_S_LGA";
static constexpr auto AR1_S_LGB = "AR1_S_LGB";
static constexpr auto AR1_S_HSN = "AR1_S_HSN";
static constexpr auto AR1_S_HGA = "AR1_S_HGA";
static constexpr auto AR1_S_HGB = "AR1_S_HGB";
static constexpr auto AR1_P_SNR = "AR1_P_SNR";
static constexpr auto AR1_P_LDPT = "AR1_P_LDPT";
static constexpr auto AR1_P_LDPN = "AR1_P_LDPN";
static constexpr auto AR1_P_GNA = "AR1_P_GNA";
static constexpr auto AR1_P_GNB = "AR1_P_GNB";
static constexpr auto AR1_P_MCS = "AR1_P_MCS";
static constexpr auto AR1_P_CHN = "AR1_P_CHN";
static constexpr auto AR1_P_PWR = "AR1_P_PWR";
static constexpr auto AR1_P_LNI = "AR1_P_LNI";
static constexpr auto AR1_P_LNF = "AR1_P_LNF";
static constexpr auto AR1_P_1TX = "AR1_P_1TX";
static constexpr auto AR1_P_TFK = "AR1_P_TFK";
static constexpr auto AR1_P_LSN = "AR1_P_LSN";
static constexpr auto AR1_P_LGA = "AR1_P_LGA";
static constexpr auto AR1_P_LGB = "AR1_P_LGB";
static constexpr auto AR1_P_HSN = "AR1_P_HSN";
static constexpr auto AR1_P_HGA = "AR1_P_HGA";
static constexpr auto AR1_P_HGB = "AR1_P_HGB";
static constexpr auto AR_V_RX_AV = "AR_V_RX_AV";
static constexpr auto AR_V_RX_OV = "AR_V_RX_OV";
static constexpr auto AR_V_RX_BS = "AR_V_RX_BS";
static constexpr auto AR_V_RX_DS = "AR_V_RX_DS";
static constexpr auto AR_V_TX_AV = "AR_V_TX_AV";
static constexpr auto AR_V_TX_OV = "AR_V_TX_OV";
static constexpr auto AR_V_TX_BS = "AR_V_TX_BS";
static constexpr auto AR_V_TX_DS = "AR_V_TX_DS";
static constexpr auto AR_T_RX_AV = "AR_T_RX_AV";
static constexpr auto AR_T_RX_OV = "AR_T_RX_OV";
static constexpr auto AR_T_RX_BS = "AR_T_RX_BS";
static constexpr auto AR_T_RX_DS = "AR_T_RX_DS";
static constexpr auto AR_T_TX_AV = "AR_T_TX_AV";
static constexpr auto AR_T_TX_OV = "AR_T_TX_OV";
static constexpr auto AR_T_TX_BS = "AR_T_TX_BS";
static constexpr auto AR_T_TX_DS = "AR_T_TX_DS";

struct ArtosynLinkSettings {
  std::string addr = "127.0.0.1";
  int port = 50000;
  int slot = 0;
  int video_port = 2;
  int telemetry_port = 1;
  int use_datagram = 1;
  int rx_buf_size = 64 * 1024;
  int tx_buf_size = 64 * 1024;
  int read_timeout_ms = 100;
  int daemon_autostart = 1;
  std::string daemon_start_cmd;
  // Link control
  int mcs_mode = 1;      // 1 auto, 0 manual
  int mcs_value = -1;    // valid when manual
  int mcs_min = -1;      // optional limit for auto
  int mcs_max = -1;      // optional limit for auto
  int bw_mode = 1;       // 1 auto, 0 manual
  int bw_value = -1;     // bb_bandwidth_e when manual
  int chan_mode = 1;     // 1 auto, 0 manual
  int chan_index = -1;   // valid when manual
  int power_auto = 1;    // 1 auto, 0 manual
  int tx_power_dbm = -1; // valid when manual, 0-31
  int band_mode = -1;    // 1 auto, 0 manual
  int band_value = -1;   // bb_band_e when manual
  int compliance_mode = -1; // 1 enable, 0 disable
  int power_mode = -1;   // bb_phy_pwr_mode_e
  int lna_mode = -1;     // 1 auto, 0 manual
  int lna_bypass = -1;   // 1 bypass, 0 lna
  int rf_a_tx = -1;      // 1 on, 0 off
  int rf_a_rx = -1;      // 1 on, 0 off
  int rf_b_tx = -1;      // 1 on, 0 off
  int rf_b_rx = -1;      // 1 on, 0 off
};

class ArtosynLinkSettingsHolder
    : public openhd::PersistentSettings<ArtosynLinkSettings> {
 public:
  ArtosynLinkSettingsHolder()
      : openhd::PersistentSettings<ArtosynLinkSettings>(
            get_interface_settings_directory()) {
    init();
  }

 protected:
  [[nodiscard]] std::string get_unique_filename() const override {
    return "artosyn_link.json";
  }
  ArtosynLinkSettings create_default() const override {
    return ArtosynLinkSettings{};
  }
  std::optional<ArtosynLinkSettings> impl_deserialize(
      const std::string& file_as_string) const override;
  std::string imp_serialize(const ArtosynLinkSettings& data) const override;
};

}  // namespace openhd

#endif  // OPENHD_ARTOSYN_LINK_SETTINGS_H
