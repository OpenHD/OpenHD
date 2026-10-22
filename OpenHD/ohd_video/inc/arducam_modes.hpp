#pragma once
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fstream>
#include <string>
#include <map>
#include <optional>
#include <cstring>

namespace arducam_modes {
struct Control { const char* id; int index, min, max, step, def; };
inline constexpr Control controls[] = {
  {"ISP_EXPOSURE_US",0,100,4000,100,1000},
  {"ISP_GAIN",1,100,800,25,100},
  {"ISP_WB_AUTO",2,0,1,1,1},
  {"ISP_WB_K",3,2000,10000,500,6500},
  {"BRIGHTNESS",4,0,200,10,100},
  {"CONTRAST",5,0,200,10,100},
  {"SATURATION",6,0,200,10,100},
  {"ISP_GAMMA",7,0,200,10,0}, // 0 selects the ISP sRGB curve.
  {"ISP_AE_MODE",8,0,1,1,1}, // Standard V4L2 auto=0, manual=1.
};
inline unsigned int native_id(const Control& c) {
  constexpr unsigned int ids[] = {V4L2_CID_EXPOSURE_ABSOLUTE,V4L2_CID_GAIN,
    V4L2_CID_AUTO_WHITE_BALANCE,V4L2_CID_WHITE_BALANCE_TEMPERATURE,
    V4L2_CID_BRIGHTNESS,V4L2_CID_CONTRAST,V4L2_CID_SATURATION,
    V4L2_CID_GAMMA,V4L2_CID_EXPOSURE_AUTO};
  return ids[c.index];
}
inline std::string sensor_node() {
  for (int i=0;i<32;++i) {
    const auto suffix=std::to_string(i);
    std::ifstream file("/sys/class/video4linux/v4l-subdev"+suffix+"/name");
    std::string name;std::getline(file,name);
    if(name.find("arducam-pivariety")!=std::string::npos)return "/dev/v4l-subdev"+suffix;
  }
  return {};
}
inline int native_value(const Control& c,int value) {
  if(c.index==0)return value/100; // V4L2 exposure_absolute uses 100 us.
  if(c.index==1)return value*256/100;
  if(c.index==4)return (value-100)*4096/100;
  if(c.index>=5 && c.index<=7)return value*4096/100;
  return value;
}
inline bool live_supported() {
  const auto node=sensor_node();if(node.empty())return false;
  const int fd=open(node.c_str(),O_RDONLY|O_CLOEXEC);if(fd<0)return false;
  v4l2_queryctrl q{};q.id=V4L2_CID_EXPOSURE_ABSOLUTE;
  const bool ok=ioctl(fd,VIDIOC_QUERYCTRL,&q)==0;
  close(fd);return ok;
}
inline bool write_control(const Control& c,int value) {
  if(value<c.min || value>c.max || (value-c.min)%c.step)return false;
  const auto node=sensor_node();if(node.empty())return false;
  const int fd=open(node.c_str(),O_RDWR|O_CLOEXEC);if(fd<0)return false;
  v4l2_control ctrl{};ctrl.id=native_id(c);
  ctrl.value=native_value(c,value);
  const bool ok=ioctl(fd,VIDIOC_S_CTRL,&ctrl)==0;close(fd);return ok;
}
inline bool restore_controls(int fd,const std::map<std::string,int>& values) {
  for(const auto& c:controls) {
    auto found=values.find(c.id);int value=found==values.end()?c.def:found->second;
    if(value<c.min || value>c.max || (value-c.min)%c.step)return false;
    v4l2_control ctrl{};ctrl.id=native_id(c);
    ctrl.value=native_value(c,value);
    if(ioctl(fd,VIDIOC_S_CTRL,&ctrl)<0)return false;
  }
  return true;
}

inline bool hdr_supported() {
  for (int i = 0; i < 32; ++i) {
    const auto suffix = std::to_string(i);
    std::ifstream file("/sys/class/video4linux/v4l-subdev" + suffix + "/name");
    std::string name; std::getline(file, name);
    if (name.find("arducam-pivariety") == std::string::npos) continue;
    const auto node = "/dev/v4l-subdev" + suffix;
    const int fd = open(node.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0) return false;
    v4l2_queryctrl ctrl{}; ctrl.id = V4L2_CID_WIDE_DYNAMIC_RANGE;
    const bool supported = ioctl(fd, VIDIOC_QUERYCTRL, &ctrl) == 0 &&
        !(ctrl.flags & (V4L2_CTRL_FLAG_DISABLED | V4L2_CTRL_FLAG_READ_ONLY)) &&
        ctrl.minimum == 0 && ctrl.maximum == 1;
    close(fd); return supported;
  }
  return false;
}
inline bool hdr_format(int width, int height, int fps) {
  return fps == 30 && ((width == 1280 && height == 720) ||
                       (width == 1920 && height == 1080));
}
}
