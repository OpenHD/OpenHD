#include "artosyn_link_settings.h"

#include "include_json.hpp"

namespace openhd {

static void to_json(nlohmann::json& j,
                    const ArtosynLinkSettings& s) {
  j = nlohmann::json{
      {"addr", s.addr},
      {"port", s.port},
      {"slot", s.slot},
      {"video_port", s.video_port},
      {"telemetry_port", s.telemetry_port},
      {"use_datagram", s.use_datagram},
      {"rx_buf_size", s.rx_buf_size},
      {"tx_buf_size", s.tx_buf_size},
      {"read_timeout_ms", s.read_timeout_ms},
      {"daemon_autostart", s.daemon_autostart},
      {"daemon_start_cmd", s.daemon_start_cmd},
      {"mcs_mode", s.mcs_mode},
      {"mcs_value", s.mcs_value},
      {"mcs_min", s.mcs_min},
      {"mcs_max", s.mcs_max},
      {"bw_mode", s.bw_mode},
      {"bw_value", s.bw_value},
      {"chan_mode", s.chan_mode},
      {"chan_index", s.chan_index},
      {"power_auto", s.power_auto},
      {"tx_power_dbm", s.tx_power_dbm},
      {"band_mode", s.band_mode},
      {"band_value", s.band_value},
      {"compliance_mode", s.compliance_mode},
      {"power_mode", s.power_mode},
      {"lna_mode", s.lna_mode},
      {"lna_bypass", s.lna_bypass},
      {"rf_a_tx", s.rf_a_tx},
      {"rf_a_rx", s.rf_a_rx},
      {"rf_b_tx", s.rf_b_tx},
      {"rf_b_rx", s.rf_b_rx},
  };
}

static void from_json(const nlohmann::json& j,
                      ArtosynLinkSettings& s) {
  s.addr = j.value("addr", s.addr);
  s.port = j.value("port", s.port);
  s.slot = j.value("slot", s.slot);
  s.video_port = j.value("video_port", s.video_port);
  s.telemetry_port = j.value("telemetry_port", s.telemetry_port);
  s.use_datagram = j.value("use_datagram", s.use_datagram);
  s.rx_buf_size = j.value("rx_buf_size", s.rx_buf_size);
  s.tx_buf_size = j.value("tx_buf_size", s.tx_buf_size);
  s.read_timeout_ms = j.value("read_timeout_ms", s.read_timeout_ms);
  s.daemon_autostart = j.value("daemon_autostart", s.daemon_autostart);
  s.daemon_start_cmd = j.value("daemon_start_cmd", s.daemon_start_cmd);
  s.mcs_mode = j.value("mcs_mode", s.mcs_mode);
  s.mcs_value = j.value("mcs_value", s.mcs_value);
  s.mcs_min = j.value("mcs_min", s.mcs_min);
  s.mcs_max = j.value("mcs_max", s.mcs_max);
  s.bw_mode = j.value("bw_mode", s.bw_mode);
  s.bw_value = j.value("bw_value", s.bw_value);
  s.chan_mode = j.value("chan_mode", s.chan_mode);
  s.chan_index = j.value("chan_index", s.chan_index);
  s.power_auto = j.value("power_auto", s.power_auto);
  s.tx_power_dbm = j.value("tx_power_dbm", s.tx_power_dbm);
  s.band_mode = j.value("band_mode", s.band_mode);
  s.band_value = j.value("band_value", s.band_value);
  s.compliance_mode = j.value("compliance_mode", s.compliance_mode);
  s.power_mode = j.value("power_mode", s.power_mode);
  s.lna_mode = j.value("lna_mode", s.lna_mode);
  s.lna_bypass = j.value("lna_bypass", s.lna_bypass);
  s.rf_a_tx = j.value("rf_a_tx", s.rf_a_tx);
  s.rf_a_rx = j.value("rf_a_rx", s.rf_a_rx);
  s.rf_b_tx = j.value("rf_b_tx", s.rf_b_tx);
  s.rf_b_rx = j.value("rf_b_rx", s.rf_b_rx);
}

std::optional<ArtosynLinkSettings>
ArtosynLinkSettingsHolder::impl_deserialize(
    const std::string& file_as_string) const {
  return openhd_json_parse<ArtosynLinkSettings>(file_as_string);
}

std::string ArtosynLinkSettingsHolder::imp_serialize(
    const ArtosynLinkSettings& data) const {
  const nlohmann::json tmp = data;
  return tmp.dump(4);
}

}  // namespace openhd
