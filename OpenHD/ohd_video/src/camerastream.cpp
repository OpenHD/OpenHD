
#include "camerastream.h"

#include <utility>

CameraStream::CameraStream(PlatformType platform,std::shared_ptr<CameraHolder> camera_holder, uint16_t video_udp_port) :
    _platform_type(platform),
    _camera_holder(std::move(camera_holder)),
    _video_udp_port(video_udp_port) {

}

