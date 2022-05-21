#ifndef OPENHD_PLATFORM_H
#define OPENHD_PLATFORM_H

#include <string>
#include <sstream>
#include <fstream>

#include "openhd-util.hpp"
#include "openhd-log.hpp"
#include "json.hpp"

typedef enum PlatformType {
  PlatformTypeRaspberryPi,
  PlatformTypeJetson,
  PlatformTypeNanoPi,
  PlatformTypeiMX6,
  PlatformTypeRockchip,
  PlatformTypeZynq,
  PlatformTypePC,
  PlatformTypeUnknown
} PlatformType;
inline std::string platform_type_to_string(PlatformType platform_type) {
  switch (platform_type) {
	case PlatformTypeJetson: return "jetson";
	case PlatformTypeRaspberryPi: return "raspberrypi";
	case PlatformTypeNanoPi: return "nanopi";
	case PlatformTypeiMX6: return "imx6";
	case PlatformTypeZynq: return "zynq";
	case PlatformTypePC: return "pc";
	default: return "unknown";
  }
}
inline PlatformType string_to_platform_type(const std::string &platform_type) {
  if (OHDUtil::to_uppercase(platform_type).find(OHDUtil::to_uppercase("jetson")) != std::string::npos) {
	return PlatformTypeJetson;
  } else if (OHDUtil::to_uppercase(platform_type).find(OHDUtil::to_uppercase("raspberrypi")) != std::string::npos) {
	return PlatformTypeRaspberryPi;
  } else if (OHDUtil::to_uppercase(platform_type).find(OHDUtil::to_uppercase("nanopi")) != std::string::npos) {
	return PlatformTypeNanoPi;
  } else if (OHDUtil::to_uppercase(platform_type).find(OHDUtil::to_uppercase("imx6")) != std::string::npos) {
	return PlatformTypeiMX6;
  } else if (OHDUtil::to_uppercase(platform_type).find(OHDUtil::to_uppercase("zynq")) != std::string::npos) {
	return PlatformTypeZynq;
  } else if (OHDUtil::to_uppercase(platform_type).find(OHDUtil::to_uppercase("pc")) != std::string::npos) {
	return PlatformTypePC;
  }
  return PlatformTypeUnknown;
}

typedef enum BoardType {
  BoardTypeRaspberryPiZero,
  BoardTypeRaspberryPiZeroW,
  BoardTypeRaspberryPi2B,
  BoardTypeRaspberryPi3A,
  BoardTypeRaspberryPi3APlus,
  BoardTypeRaspberryPi3B,
  BoardTypeRaspberryPi3BPlus,
  BoardTypeRaspberryPiCM,
  BoardTypeRaspberryPiCM3,
  BoardTypeRaspberryPiCM3Plus,
  BoardTypeRaspberryPiCM4,
  BoardTypeRaspberryPi4B,
  BoardTypeJetsonNano,
  BoardTypeJetsonTX1,
  BoardTypeJetsonTX2,
  BoardTypeJetsonNX,
  BoardTypeJetsonAGX,
  BoardTypeNanoPiNeo4,
  BoardTypePynqZ1,
  BoardTypePynqZ2,
  BoardType3DRSolo,
  BoardTypeGenericPC,
  BoardTypeUnknown
} BoardType;
inline std::string board_type_to_string(BoardType board_type) {
  switch (board_type) {
	case BoardTypeRaspberryPiZero: return "pizero";
	case BoardTypeRaspberryPiZeroW: return "pizerow";
	case BoardTypeRaspberryPi2B: return "pi2b";
	case BoardTypeRaspberryPi3A: return "pi3a";
	case BoardTypeRaspberryPi3APlus: return "pi3aplus";
	case BoardTypeRaspberryPi3B: return "pi3b";
	case BoardTypeRaspberryPi3BPlus: return "pi3bplus";
	case BoardTypeRaspberryPiCM: return "picm";
	case BoardTypeRaspberryPiCM3: return "picm3";
	case BoardTypeRaspberryPiCM3Plus: return "picm3plus";
	case BoardTypeRaspberryPiCM4: return "picm4";
	case BoardTypeRaspberryPi4B: return "pi4b";
	case BoardTypeJetsonNano: return "jetson-nano";
	case BoardTypeJetsonTX1: return "jetson-tx1";
	case BoardTypeJetsonTX2: return "jetson-tx2";
	case BoardTypeJetsonNX: return "jetson-nx";
	case BoardTypeJetsonAGX: return "jetson-agx";
	case BoardTypeNanoPiNeo4: return "nanopi-neo4";
	case BoardTypePynqZ1: return "pynqz1";
	case BoardTypePynqZ2: return "pynqz2";
	case BoardType3DRSolo: return "3dr-solo";
	case BoardTypeGenericPC: return "generic-pc";
	default: return "unknown";
  }
}
inline BoardType board_type_from_string(const std::string& s){
  if (OHDUtil::to_uppercase(s).find(OHDUtil::to_uppercase("pizero")) != std::string::npos) {
	return BoardTypeRaspberryPiZero;
  }else  if (OHDUtil::to_uppercase(s).find(OHDUtil::to_uppercase("pizerow")) != std::string::npos) {
	return BoardTypeRaspberryPiZeroW;
  }else  if (OHDUtil::to_uppercase(s).find(OHDUtil::to_uppercase("pi2b")) != std::string::npos) {
	return BoardTypeRaspberryPi2B;
  }else  if (OHDUtil::to_uppercase(s).find(OHDUtil::to_uppercase("pi3a")) != std::string::npos) {
	return BoardTypeRaspberryPi3A;
  }else  if (OHDUtil::to_uppercase(s).find(OHDUtil::to_uppercase("pi3aplus")) != std::string::npos) {
	return BoardTypeRaspberryPi3APlus;
  }else  if (OHDUtil::to_uppercase(s).find(OHDUtil::to_uppercase("pi3b")) != std::string::npos) {
	return BoardTypeRaspberryPi3B;
  }else  if (OHDUtil::to_uppercase(s).find(OHDUtil::to_uppercase("pi3bplus")) != std::string::npos) {
	return BoardTypeRaspberryPi3BPlus;
  }else  if (OHDUtil::to_uppercase(s).find(OHDUtil::to_uppercase("picm4")) != std::string::npos) {
	return BoardTypeRaspberryPiCM4;
  }else  if (OHDUtil::to_uppercase(s).find(OHDUtil::to_uppercase("pi4b")) != std::string::npos) {
	return BoardTypeRaspberryPi4B;
  }else  if (OHDUtil::to_uppercase(s).find(OHDUtil::to_uppercase("generic-pc")) != std::string::npos) {
	return BoardTypeGenericPC;
  }else  if (OHDUtil::to_uppercase(s).find(OHDUtil::to_uppercase("jetson-nano")) != std::string::npos) {
	return BoardTypeJetsonNano;
  }else{
	return BoardTypeUnknown;
  }
}

typedef enum CarrierType {
  CarrierTypeStereoPi,
  CarrierTypeComputeModuleIO,
  CarrierTypeJetsonNanoDevkit,
  CarrierTypeJetsonNCB00,
  CarrierTypeNone
} CarrierType;
inline std::string carrier_type_to_string(CarrierType carrier_type) {
  switch (carrier_type) {
	case CarrierTypeStereoPi: {
	  return "stereopi";
	}
	case CarrierTypeComputeModuleIO: {
	  return "computemoduleio";
	}
	case CarrierTypeJetsonNanoDevkit: {
	  return "jetson-nano-devkit";
	}
	case CarrierTypeJetsonNCB00: {
	  return "jetson-ncb00";
	}
	default: {
	  return "none";
	}
  }
}

// All these members must not change during run time once they have been discovered !
struct OHDPlatform {
  PlatformType platform_type = PlatformTypeUnknown;
  BoardType board_type = BoardTypeUnknown;
  CarrierType carrier_type = CarrierTypeNone;
};


// Writes the detected platform data to a json.
// This can be used for debugging, mostly a microservices artifact.
static nlohmann::json platform_to_json(const OHDPlatform &ohdPlatform) {
  nlohmann::ordered_json j;
  j["platform"] = platform_type_to_string(ohdPlatform.platform_type);
  j["board"] = board_type_to_string(ohdPlatform.board_type);
  j["carrier"] = carrier_type_to_string(ohdPlatform.carrier_type);
  return j;
}

static constexpr auto PLATFORM_MANIFEST_FILENAME = "/tmp/platform_manifest";

static void write_platform_manifest(const OHDPlatform &ohdPlatform) {
  auto manifest = platform_to_json(ohdPlatform);
  std::ofstream _t(PLATFORM_MANIFEST_FILENAME);
  _t << manifest.dump(4);
  _t.close();
}

// NOTE: It is only safe to call this method after the discovery step.
static OHDPlatform platform_from_manifest() {
  OHDPlatform ohdPlatform;
  try {
	std::ifstream f(PLATFORM_MANIFEST_FILENAME);
	nlohmann::json j;
	f >> j;
	ohdPlatform.platform_type = string_to_platform_type(j["platform"]);
	// TODO the to string for board and carrier type are still missing.
	ohdPlatform.carrier_type = CarrierTypeNone;
	ohdPlatform.board_type= board_type_from_string(j["board"]);
	//ohdPlatform.carrier_type=j["carrier"];
  } catch (std::exception &ex) {
	std::cerr << "Platform manifest processing failed: " << ex.what() << std::endl;
	ohd_log(STATUS_LEVEL_EMERGENCY, "Platform manifest processing failed");
	exit(1);
  }
  return ohdPlatform;
}

#endif
