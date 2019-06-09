
#ifndef VEYECAMERAISP_H_
#define VEYECAMERAISP_H_


typedef struct
{
  //public
   MMAL_COMPONENT_T *camera_component;    /// Pointer to the camera component
   MMAL_COMPONENT_T *isp_component;    /// Pointer to the isp component
   
   //private
   
} VEYE_CAMERA_ISP_STATE;


void veye_camera_isp_set_defaults(VEYE_CAMERA_ISP_STATE *state);

 MMAL_STATUS_T create_veye_camera_isp_component(VEYE_CAMERA_ISP_STATE *state,int cameraNum);

void destroy_veye_camera_isp_component(VEYE_CAMERA_ISP_STATE *state);


#endif


