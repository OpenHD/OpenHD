//
// Created by consti10 on 12.01.24.
//

#ifndef OPENHD_CAMERA2_H
#define OPENHD_CAMERA2_H

#include <string>

static constexpr int X_CAM_TYPE_DUMMY_SW=0; // Dummy sw picture
static constexpr int X_CAM_TYPE_USB=1; // Any USB camera
static constexpr int X_CAM_TYPE_EXTERNAL=2; // input via udp rtp
static constexpr int X_CAM_TYPE_EXTERNAL_IP=3; // input via udp rtp & file start_ip_cam.txt is created.
static constexpr int X_CAM_TYPE_RPI_MMAL=4;
static constexpr int X_CAM_TYPE_RPI_MMAL_HDMI_TO_CSI=5;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_AUTO=6;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_IMX477M=7;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_SKYMASTERHDR=8;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_SKYVISIONPRO=9;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_IMX477=10;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_IMX462=11;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_IMX327=12;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_IMX290=13;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_IMX462_LOWLIGHT_MINI=14;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_RESERVED0=15;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_RESERVED1=16;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_RESERVED2=17;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_RESERVED3=18;
static constexpr int X_CAM_TYPE_RPI_VEYE_2MP=19;
static constexpr int X_CAM_TYPE_RPI_VEYE_CSIMX307=20;
static constexpr int X_CAM_TYPE_RPI_VEYE_CSSC132=21;
static constexpr int X_CAM_TYPE_RPI_VEYE_MVCAM=22;
static constexpr int X_CAM_TYPE_RPI_VEYE_RESERVED0=23;
static constexpr int X_CAM_TYPE_RPI_VEYE_RESERVED1=24;
static constexpr int X_CAM_TYPE_CUSTOM_HARDWARE_X20=25;
static constexpr int X_CAM_TYPE_CUSTOM_HARDWARE_X20_RESERVED0=26;
static constexpr int X_CAM_TYPE_CUSTOM_HARDWARE_X20_RESERVED1=27;
static constexpr int X_CAM_TYPE_ROCKCHIP_RESERVED0=28; // reserved for future use
static constexpr int X_CAM_TYPE_ROCKCHIP_RESERVED1=29;// reserved for future use
static constexpr int X_CAM_TYPE_ROCKCHIP_RESERVED2=30;// reserved for future use
// no camera, only exists to have a default value for secondary camera (which is disabled by default).
// NOTE: The primary camera cannot be disabled !
static constexpr int X_CAM_TYPE_DISABLED=2147483647;

struct XCamera {
    int camera_type = X_CAM_TYPE_DUMMY_SW;
    // 0 for primary camera, 1 for secondary camera
    int index;
    // Only valid if camera is of type USB
    // For CSI camera(s) we in general 'know' from platform and cam type how to tell the pipeline which cam/source to use.
    std::string usb_v4l2_device_node;
    bool requires_rpi_mmal_pipeline()const{
        return camera_type==X_CAM_TYPE_RPI_MMAL_HDMI_TO_CSI || camera_type==X_CAM_TYPE_RPI_MMAL;
    }
    bool requires_rpi_libcamera_pipeline()const{
        return camera_type==X_CAM_TYPE_RPI_LIBCAMERA_AUTO || camera_type==X_CAM_TYPE_RPI_LIBCAMERA_IMX477M ||
               camera_type==X_CAM_TYPE_RPI_LIBCAMERA_SKYMASTERHDR || camera_type== X_CAM_TYPE_RPI_LIBCAMERA_SKYVISIONPRO ||
               camera_type==X_CAM_TYPE_RPI_LIBCAMERA_IMX477 || camera_type==X_CAM_TYPE_RPI_LIBCAMERA_IMX462 ||
               camera_type== X_CAM_TYPE_RPI_LIBCAMERA_IMX327 || camera_type==X_CAM_TYPE_RPI_LIBCAMERA_IMX290 ||
               camera_type== X_CAM_TYPE_RPI_LIBCAMERA_IMX462_LOWLIGHT_MINI || camera_type==X_CAM_TYPE_RPI_LIBCAMERA_RESERVED0 ||
               camera_type==X_CAM_TYPE_RPI_LIBCAMERA_RESERVED1 || camera_type==X_CAM_TYPE_RPI_LIBCAMERA_RESERVED2 ||
               camera_type==X_CAM_TYPE_RPI_LIBCAMERA_RESERVED3;
    }
    bool requires_rpi_veye_pipeline()const{
        return camera_type==X_CAM_TYPE_RPI_VEYE_2MP || camera_type==X_CAM_TYPE_RPI_VEYE_CSIMX307 ||
               camera_type==X_CAM_TYPE_RPI_VEYE_CSSC132 || camera_type==X_CAM_TYPE_RPI_VEYE_MVCAM;
    }
    bool requires_x20_cedar_pipeline()const{
        return camera_type==X_CAM_TYPE_CUSTOM_HARDWARE_X20;
    }
    bool requires_rockchip_mpp_pipeline()const{
        return camera_type==X_CAM_TYPE_ROCKCHIP_RESERVED0 || camera_type==X_CAM_TYPE_ROCKCHIP_RESERVED1 ||
               camera_type==X_CAM_TYPE_ROCKCHIP_RESERVED2;
    }
};

static bool x_is_valid_cam_type(int cam_type){
    return cam_type>=0 && cam_type<=30;
}
static std::string cam_type_as_string(int cam_type){
    return "todo";
}

#endif //OPENHD_CAMERA2_H
