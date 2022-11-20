
#include "camerastream.h"

#include <utility>

CameraStream::CameraStream(PlatformType platform,std::shared_ptr<CameraHolder> camera_holder, uint16_t video_udp_port) : m_platform_type(platform),
      m_camera_holder(std::move(camera_holder)),
      m_video_udp_port(video_udp_port) {

}

