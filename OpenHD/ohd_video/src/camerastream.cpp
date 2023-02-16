
#include "camerastream.h"

#include <utility>


CameraStream::CameraStream(PlatformType platform_type,
                           std::shared_ptr<CameraHolder> camera_holder,
                           std::shared_ptr<OHDLink> itransmit):
     m_platform_type(platform_type),
     m_camera_holder(std::move(camera_holder)),
     //m_video_udp_port(0),
     m_link_handle(std::move(itransmit)){
}
void CameraStream::stop_cleanup_restart() {}
