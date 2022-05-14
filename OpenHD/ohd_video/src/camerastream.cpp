

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>
#include <vector>

#include "camerastream.h"



CameraStream::CameraStream(PlatformType platform,Camera &camera,uint16_t video_udp_port):
        m_platform_type(platform),
        m_camera(camera),
        m_video_udp_port(video_udp_port) {}

