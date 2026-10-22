#include "ground_ae.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>
static void require(bool value) {
  if (!value) throw std::runtime_error("Ground AE check failed");
}
int main() {
  using namespace openhd::ground_ae;
  constexpr int w=160,h=100,stride=192;
  std::vector<uint8_t> image(stride*h, 16);
  for(int y=0;y<h;y++)for(int x=0;x<w;x++)image[y*stride+x]=y<h/2?235:55;
  require(meter(image.data(),image.size(),stride,w,h,0)==55);
  require(meter(image.data(),image.size(),stride,w,h,1)==55);
  require(meter(image.data(),image.size(),stride,w,h,2)==235);
  require(meter(image.data(),image.size(),stride,w,h,8)==235);
  require(meter(image.data(),image.size(),stride,w,h,10)==55);
  require(meter(image.data(),10,stride,w,h,0)==-1);
  require(meter(image.data(),image.size(),stride,w,h,4)==-1);
  // Model native AE + display gamma under a bright sky. Bias raises the
  // darker ground; the sky is permitted to clip. This is an algorithm
  // simulation, not a claim about ISP convergence speed or outdoor IQ.
  Controller controller;
  int bias=8;
  for(int i=0;i<30;i++) {
    const int ground=std::lround(55*std::pow(2.,(bias-8)/8.));
    for(int y=h/2;y<h;y++)for(int x=0;x<w;x++)image[y*stride+x]=ground;
    bias=controller.update(meter(image.data(),image.size(),stride,w,h,0),bias);
  }
  require(bias>=14 && bias<=16);
  require(controller.update(-1,bias)==bias);
  controller.reset(); bias=8;
  for(int i=0;i<30;i++)
    bias=controller.update(std::lround(190*std::pow(2.,(bias-8)/8.)),bias);
  require(bias<=3);
  controller.reset();
  require(controller.update(255,0)==0);
  controller.reset();
  require(controller.update(16,16)==16);
  std::cout << "Ground metering, orientation, synthetic sky/ground and bounded AE passed\n";
}
