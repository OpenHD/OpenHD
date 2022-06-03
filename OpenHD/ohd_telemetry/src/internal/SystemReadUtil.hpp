//
// Created by consti10 on 26.04.22.
//

#ifndef XMAVLINKSERVICE_SYSTEMREADUTIL_H
#define XMAVLINKSERVICE_SYSTEMREADUTIL_H

namespace SystemReadUtil {
// from https://github.com/OpenHD/Open.HD/blob/35b6b10fbeda43cd06bbfbd90e2daf29629c2f8a/openhd-status/src/statusmicroservice.cpp#L173
// Return the CPU load of the system the generator is running on
// Unit: Percentage ?
static int readCpuLoad() {
  int cpuload_gnd = 0;
  long double a[4];
  FILE *fp;
  try {
	fp = fopen("/proc/stat", "r");
	fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3]);
  } catch (...) {
	std::cerr << "ERROR: proc reading1" << std::endl;
	return -1;
  }
  fclose(fp);
  cpuload_gnd = (a[0] + a[1] + a[2]) / (a[0] + a[1] + a[2] + a[3]) * 100;
  return cpuload_gnd;
}

// from https://github.com/OpenHD/Open.HD/blob/35b6b10fbeda43cd06bbfbd90e2daf29629c2f8a/openhd-status/src/statusmicroservice.cpp#L165
// Return the CPU/SOC temperature of the system the generator is running on
// Unit: Degree ?
static int readTemperature() {
  int cpu_temperature = 0;
  FILE *fp;
  try {
	fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
	fscanf(fp, "%d", &cpu_temperature);
  } catch (...) {
	std::cerr << "ERROR: thermal reading" << std::endl;
	return -1;
  }
  fclose(fp);
  cpu_temperature = cpu_temperature / 1000;
  return cpu_temperature;
}

// copy and paste from QOpenHD, I think we can get the under-voltage warning on rpi this way.
static int readRpiUnderVoltError(){
  auto fp3 = fopen("/tmp/undervolt", "r");
  if (fp3 == nullptr) {
	return 0;
  }
  int undervolt_gnd = 0;
  fscanf(fp3,"%d", &undervolt_gnd);
  fclose(fp3);
  return undervolt_gnd;
}
}
#endif //XMAVLINKSERVICE_SYSTEMREADUTIL_H
