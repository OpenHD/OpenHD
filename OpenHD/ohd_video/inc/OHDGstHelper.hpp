//
// Created by consti10 on 15.05.22.
//

#ifndef OPENHD_OHDGSTHELPER_H
#define OPENHD_OHDGSTHELPER_H

#include <gst/gst.h>
#include "openhd-camera.hpp"
#include <sstream>
#include <string>
#include <fmt/format.h>

namespace OHDGstHelper{

    static void initGstreamerOrThrow(){
        GError* error = nullptr;
        if (!gst_init_check(nullptr, nullptr, &error)) {
            std::cerr << "gst_init_check() failed: " << error->message << std::endl;
            g_error_free(error);
            throw std::runtime_error("GStreamer initialization failed");
        }
    }

    // a createXXXStream function always ends wth "h164,h265 or mjpeg stream ! "
    // aka after that, onc can add a rtp encoder or similar.
    // ------------- crateXXXStream begin -------------
    static std::string createDummyStream(const VideoFormat videoFormat){
        // TODO: create dummies for h265 and jpeg
        assert(videoFormat.videoCodec==VideoCodecH264);
        std::stringstream ss;
        ss<<"videotestsrc ! ";
        // this part for some reason creates issues when used in combination with gst-launch
        //pipeline<<"'video/x-raw,format=(string)NV12,width=640,height=480,framerate=(fraction)30/1' ! ";
        ss<<fmt::format("x264enc bitrate={} tune=zerolatency key-int-max=10 ! ",5000);
        return ss.str();
    }

    static std::string createRpicamsrcStream(const std::string& bus,const int bitrate,const VideoFormat videoFormat){
        assert(videoFormat.isValid());
        assert(videoFormat.videoCodec==VideoCodecH264);
        std::stringstream ss;
        ss<< fmt::format("rpicamsrc name=bitratectrl camera-number={} bitrate={} preview=0 ! ",bus,bitrate);
        ss<< fmt::format("video/x-h264, profile=constrained-baseline, width={}, height={}, framerate={}/1, level=3.0 ! ",
                                  videoFormat.width, videoFormat.height,videoFormat.framerate);
        return ss.str();
    }

    static std::string createJetsonStream(const int sensor_id,const int bitrate,const VideoFormat videoFormat){
        std::stringstream ss;
        ss << fmt::format("nvarguscamerasrc do-timestamp=true sensor-id={} ! ", sensor_id);
        ss << fmt::format("video/x-raw(memory:NVMM), width={}, height={}, format=NV12, framerate={}/1 ! ",
                                  videoFormat.width, videoFormat.height,videoFormat.framerate);
        //ss << "queue ! ";
        if (videoFormat.videoCodec == VideoCodecH265) {
            ss << fmt::format("nvv4l2h265enc name=vnenc control-rate=1 insert-sps-pps=1 bitrate={} ! ",bitrate);
        } else if (videoFormat.videoCodec == VideoCodecMJPEG) {
            ss<< fmt::format("nvjpegenc quality=50 ! ");
        } else {
            // H264 or unknown
            ss << fmt::format("nvv4l2h264enc name=nvenc control-rate=1 insert-sps-pps=1 bitrate={} ! ",bitrate);
        }
        return ss.str();
    }

    // For Cameras that do raw YUV (or RGB) we use a sw hw encoder, which also means no h265
    static std::string createUvcRawSwEncodingStream(){
        return {};
    }

    // These are not tested

    static std::string createUVCH264Stream(const std::string& device_node,const int bitrate,const VideoFormat videoFormat){
        assert(videoFormat.videoCodec==VideoCodecH264);
        std::stringstream ss;
        ss << fmt::format("uvch264src device={} peak-bitrate={} initial-bitrate={} average-bitrate={} rate-control=1 iframe-period=1000 name=encodectrl auto-start=true encodectrl.vidsrc ! ", device_node,bitrate,bitrate,bitrate);
        ss << fmt::format("video/x-h264,width={}, height={}, framerate={}/1 ! ",
                                  videoFormat.width,videoFormat.height,videoFormat.framerate);
        return ss.str();
    }

    static std::string createIpCameraStream(const std::string& url){
        std::stringstream ss;
        // none of the other params are used at the moment, we would need to set them with ONVIF or a per-camera API of some sort,
        // however most people seem to set them in advance with the proprietary app that came with the camera anyway
        ss << fmt::format("rtspsrc location=\"{}\" latency=0 ! ", url);
        return ss.str();
    }

    // ------------- crateXXXStream end  -------------


    /**
     * Create the part of the pipeline that takes the rtp from gstreamer and sends it to udp.
     * @param udpOutPort the udp (localhost) port.
     * @return the gstreamer pipeline part
     */
    static std::string createOutputUdpLocalhost(const int udpOutPort){
        return fmt::format(" udpsink host=127.0.0.1 port={} ", udpOutPort);
    }

    /**
    * Create the part of the pipeline that takes the raw h264/h265/mjpeg from gstreamer and packs it into rtp.
    * @param videoCodec the video codec o create the rtp for.
    * @return the gstreamer pipeline part.
    */
    static std::string createRtpForVideoCodec(const VideoCodec videoCodec){
        std::stringstream ss;
        assert(videoCodec!=VideoCodecUnknown);
        if(videoCodec==VideoCodecH264){
            ss << "h264parse config-interval=-1 ! ";
            ss << "rtph264pay mtu=1024 ! ";
        }else if(videoCodec==VideoCodecH265){
            ss << "h265parse config-interval=-1 ! ";
            ss << "rtph265pay mtu=1024 ! ";
        }else{
            assert(videoCodec==VideoCodecMJPEG);
            // mjpeg
            ss << "jpegparse config-interval=-1 ! ";
            ss << "rtpjpegpay mtu=1024 ! ";
        }
        return ss.str();
    }
}
#endif //OPENHD_OHDGSTHELPER_H
