
#include "camerastream.h"

#include <utility>

CameraStream::CameraStream(PlatformType platform,std::shared_ptr<CameraHolder> camera_holder, uint16_t video_udp_port) :
      m_platform_type(platform),
      m_camera_holder(std::move(camera_holder)),
      m_video_udp_port(video_udp_port) {
}

CameraStream::CameraStream(PlatformType platform_type,
                           std::shared_ptr<CameraHolder> camera_holder,
                           std::shared_ptr<openhd::ITransmitVideo> itransmit):
     m_platform_type(platform_type),
     m_camera_holder(std::move(camera_holder)),
     m_video_udp_port(0),
     m_transmit_interface(std::move(itransmit)){
}
