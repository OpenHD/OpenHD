#ifndef OPENHD_CAMERA_H
#define OPENHD_CAMERA_H

#include <string>
#include <vector>
#include <sstream>
#include <fstream>

#include <regex>

#include "openhd-util.hpp"
#include "openhd-log.hpp"
#include "json.hpp"

typedef enum CameraType {
  CameraTypeRaspberryPiCSI, //Rpi foundation CSI camera
  CameraTypeRaspberryPiVEYE,
  CameraTypeJetsonCSI, //Any CSI camera on jetson
  CameraTypeRockchipCSI, //Any CSI camera on rockchip
  // I think this is a 44l2 camera so to say, too.
  CameraTypeUVC,
  // this is not just a UVC camera that happens to support h264, it's the standard UVC H264 that only a few cameras
  // support, like the older models of the Logitech C920. Other UVC cameras may support h264, but they do it in a
  // completely different way so we keep them separate
  CameraTypeUVCH264,
  CameraTypeIP, // IP camera that connects via ethernet and provides a video feet at special network address
  CameraTypeDummy, // Dummy camera, is created fully in sw
  CameraTypeUnknown
} CameraType;
static std::string camera_type_to_string(const CameraType &camera_type) {
  switch (camera_type) {
	case CameraTypeRaspberryPiCSI: return "pi-csi";
	case CameraTypeRaspberryPiVEYE: return "pi-veye";
	case CameraTypeJetsonCSI: return "jetson-csi";
	case CameraTypeRockchipCSI: return "rockchip-csi";
	case CameraTypeUVC: return "uvc";
	case CameraTypeUVCH264: return "uvch264";
	case CameraTypeIP: return "ip";
	default: return "unknown";
  }
}
static CameraType string_to_camera_type(const std::string &camera_type) {
  if (OHDUtil::to_uppercase(camera_type).find(OHDUtil::to_uppercase("pi-csi")) != std::string::npos) {
	return CameraTypeRaspberryPiCSI;
  } else if (OHDUtil::to_uppercase(camera_type).find(OHDUtil::to_uppercase("pi-veye")) != std::string::npos) {
	return CameraTypeRaspberryPiVEYE;
  } else if (OHDUtil::to_uppercase(camera_type).find(OHDUtil::to_uppercase("jetson-csi")) != std::string::npos) {
	return CameraTypeJetsonCSI;
  } else if (OHDUtil::to_uppercase(camera_type).find(OHDUtil::to_uppercase("rockchip-csi")) != std::string::npos) {
	return CameraTypeRockchipCSI;
  } else if (OHDUtil::to_uppercase(camera_type).find(OHDUtil::to_uppercase("uvc")) != std::string::npos) {
	return CameraTypeUVC;
  } else if (OHDUtil::to_uppercase(camera_type).find(OHDUtil::to_uppercase("uvch264")) != std::string::npos) {
	return CameraTypeUVCH264;
  } else if (OHDUtil::to_uppercase(camera_type).find(OHDUtil::to_uppercase("ip")) != std::string::npos) {
	return CameraTypeIP;
  }
  return CameraTypeUnknown;
}

typedef enum VideoCodec {
  VideoCodecH264,
  VideoCodecH265,
  VideoCodecMJPEG,
  VideoCodecUnknown
} VideoCodec;
inline std::string video_codec_to_string(VideoCodec codec) {
  switch (codec) {
	case VideoCodecH264: return "h264";
	case VideoCodecH265: return "h265";
	case VideoCodecMJPEG: return "mjpeg";
	default: return "unknown";
  }
}
inline VideoCodec string_to_video_codec(const std::string &codec) {
  if (OHDUtil::to_uppercase(codec).find(OHDUtil::to_uppercase("h264")) != std::string::npos) {
	return VideoCodecH264;
  } else if (OHDUtil::to_uppercase(codec).find(OHDUtil::to_uppercase("h265")) != std::string::npos) {
	return VideoCodecH265;
  } else if (OHDUtil::to_uppercase(codec).find(OHDUtil::to_uppercase("mjpeg")) != std::string::npos) {
	return VideoCodecMJPEG;
  }
  return VideoCodecUnknown;
}

// A video format refers to a selected configuration supported by OpenHD.
// It is possible that a camera cannot do the selected configuration in HW,
// In this case a sw encoder can be used (in case of low res streams, that will work even on the pi).
// Example string: h264|1280x720@60
// This class also provides a safe way to cast from/to a readable string. However, in case someone manually
// enters a wrong string (for example h264OOPS|1280x720@60) the behaviour is undefined.
struct VideoFormat {
  // The video codec, we default to h264
  VideoCodec videoCodec = VideoCodecH264;
  // The width of this stream, in pixels
  int width = 640;
  // The height of this stream, in pixels
  int height = 480;
  // The framerate of this stream, in frames per second.
  int framerate = 30;
  // For checking if 2 video formats are exactly the same
  bool operator==(const VideoFormat &o) const {
	return this->width == o.width && this->height == o.height && this->framerate == o.framerate;
  }
  // Return true if the Video format is valid, aka the values set "make sense".
  // values <=0 mean something went wrong during parsing or similar. And for simplicity,
  // I go with 4k and 240fps max here.
  [[nodiscard]] bool isValid() const {
	return videoCodec != VideoCodecUnknown && width > 0 && height > 0 && framerate > 0 &&
		width <= 4096 && height <= 2160 && framerate <= 240;
  }
  /**
   * Convert the VideoFormat into a readable string, in this format it can be parsed back by regex.
   * @return the video format in a readable form.
   */
  [[nodiscard]] std::string toString() const {
	std::stringstream ss;
	ss << video_codec_to_string(videoCodec) << "|" << width << "x" << height << "@" << framerate;
	return ss.str();
  }
  /**
   * Convert a readable video format string into a type-safe video format.
   * @param input the string, for example as generated above.
   * @return the video format, with the parsed values from above. On failure,
   * behaviour is undefined.
   * Note: For debugging, I use https://regex101.com/
   */
  static VideoFormat fromString(const std::string &input) {
	//We default to values that are most likely going to work, in case parsing fails.
	VideoFormat ret{};
	std::smatch result;
	const std::regex reg{R"(([\w\d\s\-\:\/]*)\|(\d*)x(\d*)\@(\d*))"};
	std::cout << "Parsing:" << input << std::endl;
	if (std::regex_search(input, result, reg)) {
	  if (result.size() == 5) {
		ret.videoCodec = string_to_video_codec(result[1]);
		ret.width = atoi(result[2].str().c_str());
		ret.height = atoi(result[3].str().c_str());
		ret.framerate = atoi(result[4].str().c_str());
		std::cout << "Parsed:" << ret.toString() << "\n";
	  } else {
		std::cout << "Video format missmatch " << result.size();
		for (int a = 0; a < result.size(); a++) {
		  std::cout << " " << a << " " << result[a] << ".";
		}
		std::cout << std::endl;
	  }
	} else {
	  std::cerr << "Video regex format failed " << input << "\n";
	}
	return ret;
  }
};

// CSI cameras don't have an endpoint,
// Since there are too many specialities as if we could generify them.
// Also, in case of CIS cameras, we don't need the raw stuff, since pretty much every
// CSI camera then has a custom hw-accelerated pipeline that produces H264/H265/MJPEG out.
// However, a UVC camera might have YUV and/or MJPEG out and requires custom encoding.
struct UvcCameraEndpoint {
  std::string device_node;
};

// A raw endpoint is for cameras that support YUV or RGB raw frames.
// Most likely, the stream is then going to do sw encoding on them.
// This way, we can handle thermal cameras for example.
struct RawEndpoint {
  std::string device_node;
  std::string bus;
  std::vector<std::string> supportedRawFormats;
};

struct CameraEndpoint {
  std::string device_node;
  std::string bus;
  bool support_h264 = false;
  bool support_h265 = false;
  bool support_mjpeg = false;
  bool support_raw = false;
  std::vector<std::string> formats;
  // Consti10: cleanup- an endpoint that supports nothing, what the heck should we do with that ;)
  [[nodiscard]] bool supports_anything() const {
	return (support_h264 || support_h265 || support_mjpeg || support_raw);
  }
};

// An encoded endpoint is for cameras that support h264,h265 or MJPEG.
// This is mostly for CSI cameras, for which we then later have a HW accelerated method
// Of generating an encoded video stream that doesn't directly talk to the underlying v4l2 device node,
// but rather uses somthing else (raspivid or libcamera, as an example).
// However, some UVC cameras also support directly encoded MJPEG or h264/h265 out. In this case, they get a encoded endpoint,too.
struct EncodedEndpoint {
  // A list of all the video formats this camera can do for generating encoded data.
  // If the list of supported formats is empty, one can assume that the camera can do anything ?
  // TODO: Or should we make it a hard requirement, and what we as develoers have not said is "feasible" for the camera
  // is all it can do ?
  std::vector<VideoFormat> supportedFormats;
};

static constexpr auto DEFAULT_BITRATE_KBITS = 5000;

struct Camera {
  CameraType type = CameraTypeUnknown;
  std::string name = "unknown";
  std::string vendor = "unknown";
  std::string vid;
  std::string pid;
  // for USB this is the bus number, for CSI it's the connector number
  std::string bus;
  // Unique index of this camera, should start at 0. The index number depends on the order the cameras were picked
  // during the discovery step.
  int index = 0;
  // Only for network cameras (CameraTypeIP) URL in the rtp:// ... or similar form
  std::string url;
  // optional, if not empty we should always use the manual pipeline and discard everything else.
  std::string manual_pipeline;
  // All the endpoints supported by this camera.
  std::vector<CameraEndpoint> endpoints;
  // These values are settings that can change dynamically at run time (non-deterministic)
  // --------------------------------------- non-deterministic begin ---------------------------------------
  // The video format selected by the user. If the user sets a video format that isn't supported
  // (for example, he might select h264|1920x1080@120 but the camera can only do 60fps)
  // The stream should default to the first available video format.
  // If no video format is available, it should default to h264|640x480@30.
  VideoFormat userSelectedVideoFormat{VideoCodecH264,640,480,30};
  // All these are for the future, and probably implemented on a best effort approach-
  // e.g. changing them does not neccessarly mean the camera supports changing them,
  // and they are too many to do it in a "check if supported" manner.
  //--
  // The bitrate the generated stream should have. Note that not all cameras / encoder support a constant bitrate,
  // and not all encoders support all bitrates, especially really low ones.
  int bitrateKBits = DEFAULT_BITRATE_KBITS;
  std::string brightness;
  std::string contrast;
  std::string sharpness;
  std::string rotate;
  std::string wdr;
  std::string denoise;
  std::string thermal_palette;
  std::string thermal_span;
  // --------------------------------------- end ---------------------------------------
};

// TODO: Why the heck did stephen not use the endpoints member variable here ?
static nlohmann::json cameras_to_json(const std::vector<Camera> &cameras) {
  nlohmann::json j;
  for (const auto &camera: cameras) {
	try {
	  nlohmann::json endpoints = nlohmann::json::array();
	  int endpoint_index = 0;
	  for (const auto &_endpoint: camera.endpoints) {
		// Now this is for safety, the code by stephen was buggy in this regard
		// Aka why the heck should a camera have endpoints that are not even related to it ???!!
		// If this assertion fails, we need to check the discovery step.
		assert(camera.bus == _endpoint.bus);
		// also, a camera without a endpoint - what the heck should that be
		if (camera.endpoints.empty()) {
		  std::cerr << "to json Warning Camera without endpoints\n";
		}
		endpoints[endpoint_index] = {
			{"device_node", _endpoint.device_node},
			{"support_h264", _endpoint.support_h264},
			{"support_h265", _endpoint.support_h265},
			{"support_mjpeg", _endpoint.support_mjpeg},
			{"support_raw", _endpoint.support_raw},
			{"formats", _endpoint.formats}
		};
		endpoint_index++;
	  }
	  nlohmann::json _camera = {
		  {"type", camera_type_to_string(camera.type)},
		  {"name", camera.name},
		  {"vendor", camera.vendor},
		  {"vid", camera.vid},
		  {"pid", camera.pid},
		  {"bus", camera.bus},
		  {"index", camera.index},
		  {"url", camera.url},
		  {"manual_pipeline", camera.manual_pipeline},
		  {"endpoints", endpoints},
		  {"userSelectedVideoFormat", camera.userSelectedVideoFormat.toString()},
		  {"bitrateKBits", std::to_string(camera.bitrateKBits)}
	  };
	  std::stringstream message;
	  message << "Detected camera: " << camera.name << std::endl;
	  ohd_log(STATUS_LEVEL_INFO, message.str());
	  j.push_back(_camera);
	} catch (std::exception &ex) {
	  std::cerr << "exception: " << ex.what() << std::endl;
	}
  }
  return j;
}

static constexpr auto CAMERA_MANIFEST_FILENAME = "/tmp/camera_manifest";

static void write_camera_manifest(const std::vector<Camera> &cameras) {
  auto manifest = cameras_to_json(cameras);
  std::ofstream _t(CAMERA_MANIFEST_FILENAME);
  _t << manifest.dump(4);
  _t.close();
}

static std::vector<Camera> cameras_from_manifest() {
  std::vector<Camera> ret;
  std::cout << "Processing camera_manifest" << std::endl;
  try {
	std::ifstream f(CAMERA_MANIFEST_FILENAME);
	nlohmann::json j;
	f >> j;
	for (auto _camera: j) {
	  Camera camera;
	  std::string camera_type = _camera["type"];
	  camera.type = string_to_camera_type(camera_type);
	  camera.name = _camera["name"];
	  std::cout << camera.name << std::endl;
	  camera.vendor = _camera["vendor"];
	  camera.vid = _camera["vid"];
	  camera.pid = _camera["pid"];
	  camera.bus = _camera["bus"];
	  camera.index = _camera["index"];
	  camera.url = _camera["url"];
	  camera.manual_pipeline = _camera["manual_pipeline"];
	  auto _endpoints = _camera["endpoints"];
	  for (auto _endpoint: _endpoints) {
		CameraEndpoint endpoint;
		endpoint.device_node = _endpoint["device_node"];
		endpoint.support_h264 = _endpoint["support_h264"];
		endpoint.support_h265 = _endpoint["support_h265"];
		endpoint.support_mjpeg = _endpoint["support_mjpeg"];
		endpoint.support_raw = _endpoint["support_raw"];
		for (auto &format: _endpoint["formats"]) {
		  endpoint.formats.push_back(format);
		  std::cout << format << std::endl;
		}
		camera.endpoints.push_back(endpoint);
	  }
	  camera.userSelectedVideoFormat = VideoFormat::fromString(_camera["userSelectedVideoFormat"]);
	  const std::string bitrateKBits = _camera["bitrateKBits"];
	  camera.bitrateKBits = atoi(bitrateKBits.c_str());
	  ret.push_back(camera);
	}
  } catch (std::exception &ex) {
	// don't do anything, but send an error message to the user through the status service
	std::cerr << "Camera error: " << ex.what() << std::endl;
  }
  std::cout << "Done processing camera manifest\n";
  return ret;
}

// Return true if the bitrate is considered sane, false otherwise
static bool check_bitrate_sane(const int bitrateKBits) {
  if (bitrateKBits <= 100 || bitrateKBits > (1024 * 1024 * 50)) {
	return false;
  }
  return true;
}

#endif
