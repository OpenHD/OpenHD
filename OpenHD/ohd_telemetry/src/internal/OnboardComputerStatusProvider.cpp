//
// Created by consti10 on 30.10.22.
//

#include "OnboardComputerStatusProvider.h"

#include "onboard_computer_status.hpp"
#include "onboard_computer_status_rpi.hpp"
#include "openhd_util_filesystem.h"

//INA219 stuff
constexpr float SHUNT_OHMS = 0.1f;
constexpr float MAX_EXPECTED_AMPS = 3.2f;
constexpr uint16_t RANGE = RANGE_32V;
constexpr uint8_t GAIN = GAIN_8_320MV;
constexpr uint8_t BUS_ADC = ADC_12BIT;
constexpr uint8_t SHUNT_ADC = ADC_12BIT;
//INA219 stuff

int ohd_platform_pre;
int ohd_platform_det;
int ohd_platform;
int ohd_wifi_pre;
int ohd_wifi_det;
int ohd_wifi;
int ohd_cam_pre;
int ohd_cam_det;
int ohd_cam;
int ohd_ident;

struct CamConfiguration {
    int pre;
    int det;
};



OnboardComputerStatusProvider::OnboardComputerStatusProvider(OHDPlatform platform,bool enable)
    : m_platform(platform),
      m_enable(enable),
      m_ina_219(SHUNT_OHMS, MAX_EXPECTED_AMPS)
{
  ina219_log_warning_once();
  if(!m_ina_219.has_any_error){
    m_ina_219.configure(RANGE, GAIN, BUS_ADC, SHUNT_ADC);
  }
  if(m_enable){
    m_calculate_cpu_usage_thread=std::make_unique<std::thread>(&OnboardComputerStatusProvider::calculate_cpu_usage_until_terminate, this);
    m_calculate_other_thread=std::make_unique<std::thread>(&OnboardComputerStatusProvider::calculate_other_until_terminate, this);
  }
}

OnboardComputerStatusProvider::~OnboardComputerStatusProvider() {
  if(m_enable){
    terminate= true;
    m_calculate_cpu_usage_thread->join();
    m_calculate_other_thread->join();
  }
}

mavlink_onboard_computer_status_t OnboardComputerStatusProvider::get_current_status() {
  std::lock_guard<std::mutex> lock(m_curr_onboard_computer_status_mutex);
  return m_curr_onboard_computer_status;
}

void OnboardComputerStatusProvider::calculate_cpu_usage_until_terminate() {
  while (!terminate){
    const auto before=std::chrono::steady_clock::now();
    const auto value=openhd::onboard::read_cpuload_once_blocking();
    const auto read_time=std::chrono::steady_clock::now()-before;
    if(value.has_value()){
      // lock mutex and write out
      std::lock_guard<std::mutex> lock(m_curr_onboard_computer_status_mutex);
      m_curr_onboard_computer_status.cpu_cores[0]=value.value();
    }
    //std::cout<<"Took:"<<std::chrono::duration_cast<std::chrono::milliseconds>(read_time).count()<<"\n";
    // top can block up to X seconds, but in case it doesn't make sure we don't neccessarily waste cpu here
    const auto minimum_delay=std::chrono::seconds(1);
    if(read_time<minimum_delay){
      std::this_thread::sleep_for((minimum_delay-read_time));
    }
  }
}

void OnboardComputerStatusProvider::calculate_other_until_terminate() {
  while (!terminate){
    // We always sleep for 1 second
    // just to make sure to not hog too much cpu here.
    std::this_thread::sleep_for(std::chrono::seconds(1));
    int8_t curr_temperature_core=0;
    int curr_clock_cpu=0;
    int curr_clock_isp=0;
    int curr_clock_h264=0;
    int curr_clock_core=0;
    int curr_clock_v3d=0;
    bool curr_rpi_undervolt= false;
    int curr_ina219_voltage=0;
    int curr_ina219_current=0;

    //detect platform
    const std::string platform_paths[] = {
        "/usr/local/share/openhd_platform/x20",
        "/usr/local/share/openhd_platform/x86",
        "/usr/local/share/openhd_platform/rpi",
        "/usr/local/share/openhd_platform/rock"
    };

    const std::string rpi_subpaths[] = {
        "/usr/local/share/openhd_platform/rpi/4",
        "/usr/local/share/openhd_platform/rpi/3",
        "/usr/local/share/openhd_platform/rpi/2",
        "/usr/local/share/openhd_platform/rpi/0"
    };

    const std::string rock_subpaths[] = {
        "/usr/local/share/openhd_platform/rock/rk3566",
        "/usr/local/share/openhd_platform/rock/rock5a",
        "/usr/local/share/openhd_platform/rock/rock5b"
    };

    const std::string wifi_paths[] = {
    "/usr/local/share/openhd_platform/wifi_card_type/88xxau/ac56",
    "/usr/local/share/openhd_platform/wifi_card_type/88xxau/custom",
    "/usr/local/share/openhd_platform/wifi_card_type/88xxbu/custom",
    };

    if (OHDFilesystemUtil::exists("/usr/local/share/openhd_platform/unconfigured")) {
      for (int i = 0; i < sizeof(platform_paths) / sizeof(platform_paths[0]); i++) {
          if (OHDFilesystemUtil::exists(platform_paths[i])) {
              ohd_platform_pre = i + 1;
              if (i == 2) { // RPI platform
                  for (int j = 0; j < sizeof(rpi_subpaths) / sizeof(rpi_subpaths[0]); j++) {
                      if (OHDFilesystemUtil::exists(rpi_subpaths[j])) {
                          ohd_platform_det = j + 1;
                          break;
                      }
                  }
              } else if (i == 3) { // Rock platform
                  for (int j = 0; j < sizeof(rock_subpaths) / sizeof(rock_subpaths[0]); j++) {
                      if (OHDFilesystemUtil::exists(rock_subpaths[j])) {
                          ohd_platform_det = j + 1;
                          break;
                      }
                  }
              }
              break;
          }
      }

      // Detect wifi
    for (int i = 0; i < sizeof(wifi_paths) / sizeof(wifi_paths[0]); i++) {
      if (OHDFilesystemUtil::exists(wifi_paths[i])) {
          if (i == 0) {
              ohd_wifi_pre = 1;
              ohd_wifi_det = 1;
          } else if (i == 1) {
              ohd_wifi_pre = 1;
              ohd_wifi_det = 2;
          } else if (i == 2) {
              ohd_wifi_pre = 2;
              ohd_wifi_det = 1;
          }
          break;
        }
    }
      // Detect cam
    const std::map<std::string, CamConfiguration> cam_paths = {
      {"/usr/local/share/openhd_platform/x20/sharkbyte/runcamNano", {1, 1}},
      {"/usr/local/share/openhd_platform/x20/hdzero", {1, 0}},
      {"/usr/local/share/openhd_platform/x86/generic/generic", {1, 1}},
      {"/usr/local/share/openhd_platform/x86/generic2", {2, 0}},
      {"/usr/local/share/openhd_platform/rpi/raspivid/OV5647", {1, 1}},
      {"/usr/local/share/openhd_platform/rpi/raspivid/imx219", {1, 2}},
      {"/usr/local/share/openhd_platform/rpi/raspivid/imx477", {1, 3}},
      {"/usr/local/share/openhd_platform/rpi/raspivid/hdmi", {1, 4}},
      {"/usr/local/share/openhd_platform/rpi/libcamera/IMX462m", {2, 1}},
      {"/usr/local/share/openhd_platform/rpi/libcamera/IMX477m", {2, 2}},
      {"/usr/local/share/openhd_platform/rpi/libcamera/IMX708", {2, 3}},
      {"/usr/local/share/openhd_platform/rpi/libcamera/IMX519", {2, 4}},
      {"/usr/local/share/openhd_platform/rpi/libcamera/IMX477", {2, 5}},
      {"/usr/local/share/openhd_platform/rpi/libcamera/IMX462", {2, 6}},
      {"/usr/local/share/openhd_platform/rpi/libcamera/IMX327", {2, 7}},
      {"/usr/local/share/openhd_platform/rpi/libcamera/IMX290", {2, 8}},
      {"/usr/local/share/openhd_platform/rpi/libcamera/AUTO", {2, 9}},
      {"/usr/local/share/openhd_platform/rpi/veye/2MP", {3, 1}},
      {"/usr/local/share/openhd_platform/rpi/veye/CSIMX307", {3, 2}},
      {"/usr/local/share/openhd_platform/rpi/veye/CSSC132", {3, 3}},
      {"/usr/local/share/openhd_platform/rpi/veye/MVCAM", {3, 4}},
      {"/usr/local/share/openhd_platform/rock/rock5/imx219", {1, 1}},
      {"/usr/local/share/openhd_platform/rock/rock5/imx462", {1, 2}},
      {"/usr/local/share/openhd_platform/rock/rock5/imx708", {1, 3}},
      {"/usr/local/share/openhd_platform/rock/rock5/hdmi", {1, 4}},
      {"/usr/local/share/openhd_platform/rock/rk3566/IMX219", {2, 1}},
      {"/usr/local/share/openhd_platform/rock/rk3566/IMX708", {2, 2}},
    };

    for (const auto& path_config : cam_paths) {
        if (OHDFilesystemUtil::exists(path_config.first)) {
            ohd_cam_pre = path_config.second.pre;
            ohd_cam_det = path_config.second.det;
            break;
        }
    }

    std::string fileContents = OHDFilesystemUtil::read_file("/usr/local/share/openhd_platform/id");
    std::istringstream iss(fileContents);
      if (!(iss >> ohd_ident)) {
          std::cerr << "Error: ID file is corrupt!" << std::endl;
      }
     ohd_platform = ohd_platform_pre * 10 + ohd_platform_det;
     ohd_cam = ohd_cam_pre * 10 + ohd_cam_det;
     ohd_wifi = ohd_wifi_pre * 10 + ohd_wifi_det;

     OHDFilesystemUtil::remove_if_existing("/usr/local/share/openhd_platform/unconfigured");
    }

    const int curr_space_left=OHDFilesystemUtil::get_remaining_space_in_mb();
    const auto curr_ram_usage=openhd::onboard::calculate_memory_usage_percent();
    ina219_log_warning_once();
    if(!m_ina_219.has_any_error){
      float voltage = roundf(m_ina_219.voltage() * 1000);
      float current = roundf(m_ina_219.current() * 1000) / 1000;
      curr_ina219_voltage=voltage;
      curr_ina219_current=current;
    }
    if(m_platform.platform_type==PlatformType::RaspberryPi){
      curr_temperature_core=(int8_t)openhd::onboard::rpi::read_temperature_soc_degree();
      // temporary, until we have our own message
      curr_clock_cpu=openhd::onboard::rpi::read_curr_frequency_mhz(openhd::onboard::rpi::VCGENCMD_CLOCK_CPU);
      curr_clock_isp=openhd::onboard::rpi::read_curr_frequency_mhz(openhd::onboard::rpi::VCGENCMD_CLOCK_ISP);
      curr_clock_h264=openhd::onboard::rpi::read_curr_frequency_mhz(openhd::onboard::rpi::VCGENCMD_CLOCK_H264);
      curr_clock_core=openhd::onboard::rpi::read_curr_frequency_mhz(openhd::onboard::rpi::VCGENCMD_CLOCK_CORE);
      curr_clock_v3d=openhd::onboard::rpi::read_curr_frequency_mhz(openhd::onboard::rpi::VCGENCMD_CLOCK_V3D);
      curr_rpi_undervolt=openhd::onboard::rpi::vcgencmd_get_undervolt();
    }else{
      const auto cpu_temp=(int8_t)openhd::onboard::readTemperature();
      curr_temperature_core=cpu_temp;
    }
    {
      // lock mutex and write out
      std::lock_guard<std::mutex> lock(m_curr_onboard_computer_status_mutex);
      m_curr_onboard_computer_status.temperature_core[0]=curr_temperature_core;
      // temporary, until we have our own message
      m_curr_onboard_computer_status.storage_type[0]=curr_clock_cpu;
      m_curr_onboard_computer_status.storage_type[1]=curr_clock_isp;
      m_curr_onboard_computer_status.storage_type[2]=curr_clock_h264;
      m_curr_onboard_computer_status.storage_type[3]=curr_clock_core;
      m_curr_onboard_computer_status.storage_usage[0]=curr_clock_v3d;
      m_curr_onboard_computer_status.storage_usage[1]=curr_space_left;
      m_curr_onboard_computer_status.storage_usage[2]=curr_ina219_voltage;
      m_curr_onboard_computer_status.storage_usage[3]=curr_ina219_current;
      // openhd status message
      m_curr_onboard_computer_status.link_type[0] = ohd_platform;
      m_curr_onboard_computer_status.link_type[1] = ohd_wifi;
      m_curr_onboard_computer_status.link_type[2] = ohd_cam;
      m_curr_onboard_computer_status.link_type[3] = ohd_ident;
      m_curr_onboard_computer_status.ram_usage=static_cast<uint32_t>(curr_ram_usage.ram_usage_perc);
      m_curr_onboard_computer_status.ram_total=curr_ram_usage.ram_total_mb;
      m_curr_onboard_computer_status.link_tx_rate[0]=curr_rpi_undervolt ? 1 : 0;
    }
  }
}

MavlinkMessage
OnboardComputerStatusProvider::get_current_status_as_mavlink_message(const uint8_t sys_id,const uint8_t comp_id,
                                                                     const std::optional<ExtraUartInfo>& extra_uart_opt) {
  MavlinkMessage msg;
  auto tmp=get_current_status();
  if(extra_uart_opt.has_value()){
    const auto& extra_uart=extra_uart_opt.value();
    tmp.fan_speed[0]=extra_uart.fc_sys_id;
    tmp.fan_speed[1]=extra_uart.operating_mode;
  }
  mavlink_msg_onboard_computer_status_encode(sys_id,comp_id,&msg.m,&tmp);
  return msg;
}

void OnboardComputerStatusProvider::ina219_log_warning_once() {
  if(m_ina_219.has_any_error && !m_ina219_warning_logged){
    openhd::log::get_default()->warn("INA219 failed - no power monitoring");
    m_ina219_warning_logged= true;
  }
}
