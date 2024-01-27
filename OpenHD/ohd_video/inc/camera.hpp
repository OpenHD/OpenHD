//
// Created by consti10 on 12.01.24.
//

#ifndef OPENHD_CAMERA_HPP
#define OPENHD_CAMERA_HPP

#include <string>
#include <sstream>
#include <vector>

// For development, always 'works' since fully emulated in SW.
static constexpr int X_CAM_TYPE_DUMMY_SW=0; // Dummy sw picture
// OpenHD supports any usb camera outputting raw video (with sw encoding).
// H264 usb cameras are not supported, since in general, they do not support changing bitrate/ encoding parameters.
static constexpr int X_CAM_TYPE_USB=1;
// Manually feed camera data (encoded,rtp) to openhd. Bitrate control and more is not working in this mode,
// making it only valid for development and in extreme cases valid for users that want to use a specific ip camera.
static constexpr int X_CAM_TYPE_EXTERNAL=2;
// For openhd, this is exactly the same as X_CAM_TYPE_EXTERNAL - only file start_ip_cam.txt is created
// Such that the ip cam service can start forwarding data to openhd core.
static constexpr int X_CAM_TYPE_EXTERNAL_IP=3;
// For development, camera that reads input from a file, and then re-encodes it using the platform encoder
static constexpr int X_CAM_TYPE_DEVELOPMENT_FILESRC=4;
//
// RPI Specific starts here
//
// As of now, we have mmal only for the geekworm hdmi to csi adapter
static constexpr int X_CAM_TYPE_RPI_MMAL_HDMI_TO_CSI=10;
// ... 9 reserved for future use
// ...
// RPIF stands for RPI Foundation (aka original rpi foundation cameras)
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_RPIF_V1_OV5647=20;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_RPIF_V2_IMX219=21;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_RPIF_V3_IMX708=22;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_RPIF_HQ_IMX477=23;
// .... 5 reserved for future use
// Now to all the rpi libcamera arducam cameras (they start at 20)
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_SKYMASTERHDR=30;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_SKYVISIONPRO=31;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX477M=32;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX462=33;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX327=34;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX290=35;
static constexpr int X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX462_LOWLIGHT_MINI=36;
// ... 13 reserved for future use
static constexpr int X_CAM_TYPE_RPI_V4L2_VEYE_2MP=50;
static constexpr int X_CAM_TYPE_RPI_V4L2_VEYE_CSIMX307=51;
static constexpr int X_CAM_TYPE_RPI_V4L2_VEYE_CSSC132=52;
static constexpr int X_CAM_TYPE_RPI_V4L2_VEYE_MVCAM=53;
// ... 6 reserved for future use
//
// X20 Specific starts here
//
// Right now we only have one camera, but more (might) follow.
static constexpr int X_CAM_TYPE_X20_RUNCAM_NANO=60;
// ... 9 reserved for future use
//
// ROCK Specific starts here
//
static constexpr int X_CAM_TYPE_ROCK_HDMI_IN=70;
static constexpr int X_CAM_TYPE_ROCK_IMX219=71;
// ... rest is reserved for future use
// no camera, only exists to have a default value for secondary camera (which is disabled by default).
// NOTE: The primary camera cannot be disabled !
static constexpr int X_CAM_TYPE_DISABLED=255; // Max for uint8_t

struct XCamera {
    int camera_type = X_CAM_TYPE_DUMMY_SW;
    // 0 for primary camera, 1 for secondary camera
    int index;
    // Only valid if camera is of type USB
    // For CSI camera(s) we in general 'know' from platform and cam type how to tell the pipeline which cam/source to use.
    std::string usb_v4l2_device_node;
    bool requires_rpi_mmal_pipeline()const{
        return camera_type==X_CAM_TYPE_RPI_MMAL_HDMI_TO_CSI;
    }
    bool requires_rpi_libcamera_pipeline()const{
        return camera_type>=20 && camera_type<50;
    }
    bool requires_rpi_veye_pipeline()const{
        return camera_type>=50 && camera_type<60;
    }
    bool requires_x20_cedar_pipeline()const{
        return camera_type>=60 && camera_type<70;
    }
    bool requires_rockchip_mpp_pipeline()const{
        return camera_type>=70 && camera_type<80;
    }
    std::string cam_type_as_verbose_string()const{
        switch (camera_type) {
            case X_CAM_TYPE_DUMMY_SW: return "DUMMY";
            case X_CAM_TYPE_USB: return "USB";
            case X_CAM_TYPE_EXTERNAL: return "EXTERNAL";
            case X_CAM_TYPE_EXTERNAL_IP: return "EXTERNAL_IP";
            case X_CAM_TYPE_DEVELOPMENT_FILESRC: return "DEV_FILESRC";
            // All the rpi stuff begin
            case X_CAM_TYPE_RPI_MMAL_HDMI_TO_CSI: return "MMAL_HDMI";
            case X_CAM_TYPE_RPI_LIBCAMERA_RPIF_V1_OV5647: return "RPIF_V1_OV5647";
            case X_CAM_TYPE_RPI_LIBCAMERA_RPIF_V2_IMX219: return "RPIF_V2_IMX219";
            case X_CAM_TYPE_RPI_LIBCAMERA_RPIF_V3_IMX708: return "RPIF_V3_IMX708";
            case X_CAM_TYPE_RPI_LIBCAMERA_RPIF_HQ_IMX477: return "RPIF_HQ_IMX477";
            case X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_SKYMASTERHDR: return "ARDUCAM_SKYMASTERHDR";
            case X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_SKYVISIONPRO: return "ARDUCAM_SKYVISIONPRO";
            case X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX477M: return "ARDUCAM_IMX477M";
            case X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX462: return "ARDUCAM_IMX462";
            case X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX327: return "ARDUCAM_IMX327";
            case X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX290: return "ARDUCAM_IMX290";
            case  X_CAM_TYPE_RPI_LIBCAMERA_ARDUCAM_IMX462_LOWLIGHT_MINI: return "ARDUCAM_IMX462_LOWLIGHT_MINI";
            case X_CAM_TYPE_RPI_V4L2_VEYE_2MP: return "VEYE_2MP";
            case X_CAM_TYPE_RPI_V4L2_VEYE_CSIMX307: return "VEYE_IMX307";
            case X_CAM_TYPE_RPI_V4L2_VEYE_CSSC132: return "VEYE_CSSC132";
            case X_CAM_TYPE_RPI_V4L2_VEYE_MVCAM: return "VEYE_MVCAM";
            // All the x20 begin
            case X_CAM_TYPE_X20_RUNCAM_NANO: return "X20_RUNCAM_NANO";
            // All the rock begin
            case X_CAM_TYPE_ROCK_HDMI_IN:return "ROCK_HDMI_IN";
            case X_CAM_TYPE_ROCK_IMX219: return "ROCK_IMX219";
            case X_CAM_TYPE_DISABLED: return "DISABLED";
            default:break;
        }
        std::stringstream ss;
        ss<<"UNKNOWN ("<<camera_type<<")";
        return ss.str();
    }
    struct ResolutionFramerate{
        int width_px;
        int height_px;
        int fps;
    };
    [[nodiscard]] ResolutionFramerate get_default_resolution_fps()const{
        if(requires_rpi_veye_pipeline()){
            // Veye camera(s) only do 1080p30
            return {1920,1080,30};
        }else if(requires_x20_cedar_pipeline()){
            return {1280,720,60};
        }else if(requires_rpi_libcamera_pipeline()){
            return {1920,1080,30};
        }else if(camera_type==X_CAM_TYPE_USB){
            // TODO properly
            return {640,480,30};
        }else if(camera_type==X_CAM_TYPE_DUMMY_SW){
            return {640,480,30};
        }
        return {1920,1080,30};
    }
    // Returns a list of known supported resolution(s).
    // The first element is what openhd uses as default.
    // Must always return at least one resolution
    // Might not return all resolutions a camera supports per HW
    // (In qopenhd, we have the experiment checkbox, where the user can enter anything he likes)
    std::vector<ResolutionFramerate> get_supported_resolutions()const{
        if(requires_rpi_veye_pipeline()){
            // Veye camera(s) only do 1080p30
            return {ResolutionFramerate{1920,1080,30}};
        }else if(requires_x20_cedar_pipeline()){
            // also easy, 720p60 only (for now)
            return {ResolutionFramerate{1280,720,60}};
        } else if(camera_type==X_CAM_TYPE_USB){ // TODO properly
            return {ResolutionFramerate{640,480,30}};
        }else if(requires_rpi_libcamera_pipeline()){// TODO properly
            std::vector<ResolutionFramerate> ret;
            ret.push_back(ResolutionFramerate{1920,1080,30});
            ret.push_back(ResolutionFramerate{1280,720,60});
            ret.push_back(ResolutionFramerate{640,480,60});
            return ret;
        }else if(camera_type==X_CAM_TYPE_RPI_MMAL_HDMI_TO_CSI){
            std::vector<ResolutionFramerate> ret;
            ret.push_back(ResolutionFramerate{1920,1080,30});
            ret.push_back(ResolutionFramerate{1280,720,30});
            ret.push_back(ResolutionFramerate{1280,720,60});
            return ret;
        }
        // Not mapped yet
        return {ResolutionFramerate{1920,1080,30}};
    }
};



static bool is_valid_primary_cam_type(int cam_type){
    if(cam_type>=0 && cam_type<X_CAM_TYPE_DISABLED)return true;
    return false;
}
static bool is_valid_secondary_cam_type(int cam_type){
    if(cam_type==X_CAM_TYPE_DUMMY_SW || cam_type==X_CAM_TYPE_USB || cam_type==X_CAM_TYPE_EXTERNAL ||
        cam_type==X_CAM_TYPE_EXTERNAL_IP || cam_type==X_CAM_TYPE_DISABLED){
        return true;
    }
    return false;
}

#endif //OPENHD_CAMERA_HPP
