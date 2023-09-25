
#include "camerastream.h"

#include <utility>


CameraStream::CameraStream(PlatformType platform_type,
                           std::shared_ptr<CameraHolder> camera_holder,openhd::ON_ENCODE_FRAME_CB out_cb):
     m_platform_type(platform_type),
     m_camera_holder(std::move(camera_holder)),
     //m_video_udp_port(0),
     m_output_cb(std::move(out_cb)){
}
