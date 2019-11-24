/*
Copyright (c) 2014, DSP Group Ltd
Copyright (c) 2014, James Hughes
Copyright (c) 2013, Broadcom Europe Ltd

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * \file RaspiVidYUV.c
 * Command line program to capture a camera video stream and save file
 * as uncompressed YUV420 data
 * Also optionally display a preview/viewfinder of current camera input.
 *
 * \date 7th Jan 2014
 * \Author: James Hughes
 *
 * Description
 *
  * 2 components are created; camera and preview.
 * Camera component has three ports, preview, video and stills.
 * Preview is connected using standard mmal connections, the video output
 * is written straight to the file in YUV 420 format via the requisite buffer
 * callback. Still port is not used
 *
 * We use the RaspiCamControl code to handle the specific camera settings.
 * We use the RaspiPreview code to handle the generic preview
*/

// We use some GNU extensions (basename)
#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <sysexits.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define VERSION_STRING "v1.3.15"

#include "bcm_host.h"
#include "interface/vcos/vcos.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"

#include "RaspiCamControl.h"
#include "RaspiPreview.h"
#include "RaspiCLI.h"

#include <semaphore.h>

#include "VeyeCameraIsp.h"

// Standard port setting for the camera component
#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2

// Video format information
// 0 implies variable
#define VIDEO_FRAME_RATE_NUM 30
#define VIDEO_FRAME_RATE_DEN 1

/// Video render needs at least 2 buffers.
#define VIDEO_OUTPUT_BUFFERS_NUM 3

/// Interval at which we check for an failure abort during capture
const int ABORT_INTERVAL = 100; // ms


/// Capture/Pause switch method
/// Simply capture for time specified
#define WAIT_METHOD_NONE           0
/// Cycle between capture and pause for times specified
#define WAIT_METHOD_TIMED          1
/// Switch between capture and pause on keypress
#define WAIT_METHOD_KEYPRESS       2
/// Switch between capture and pause on signal
#define WAIT_METHOD_SIGNAL         3
/// Run/record forever
#define WAIT_METHOD_FOREVER        4

int mmal_status_to_int(MMAL_STATUS_T status);
static void signal_handler(int signal_number);

// Forward
typedef struct RASPIVIDYUV_STATE_S RASPIVIDYUV_STATE;

/** Struct used to pass information in camera video port userdata to callback
 */
typedef struct
{
   FILE *file_handle;                   /// File handle to write buffer data to.
   RASPIVIDYUV_STATE *pstate;           /// pointer to our state in case required in callback
   int abort;                           /// Set to 1 in callback if an error occurs to attempt to abort the capture
   FILE *pts_file_handle;               /// File timestamps
   int frame;
   int64_t starttime;
   int64_t lasttime;
} PORT_USERDATA;

/** Structure containing all state information for the current run
 */
struct RASPIVIDYUV_STATE_S
{
   int timeout;                        /// Time taken before frame is grabbed and app then shuts down. Units are milliseconds
   int width;                          /// Requested width of image
   int height;                         /// requested height of image
   int framerate;                      /// Requested frame rate (fps)
   char *filename;                     /// filename of output file
   int verbose;                        /// !0 if want detailed run information
   int demoMode;                       /// Run app in demo mode
   int demoInterval;                   /// Interval between camera settings changes
   int waitMethod;                     /// Method for switching between pause and capture

   int onTime;                         /// In timed cycle mode, the amount of time the capture is on per cycle
   int offTime;                        /// In timed cycle mode, the amount of time the capture is off per cycle

   int onlyLuma;                       /// Only output the luma / Y plane of the YUV data
   int useRGB;                         /// Output RGB data rather than YUV

   //RASPIPREVIEW_PARAMETERS preview_parameters;   /// Preview setup parameters
   //RASPICAM_CAMERA_PARAMETERS camera_parameters; /// Camera setup parameters
    VEYE_CAMERA_ISP_STATE	veye_camera_isp_state;
   MMAL_CONNECTION_T *isp_connection; /// Pointer to the connection from isp to camera

 //  MMAL_COMPONENT_T *camera_component;    /// Pointer to the camera component
//   MMAL_CONNECTION_T *preview_connection; /// Pointer to the connection from camera to preview

   MMAL_POOL_T *camera_pool;            /// Pointer to the pool of buffers used by camera video port

   PORT_USERDATA callback_data;         /// Used to move data to the camera callback

   int bCapturing;                      /// State of capture/pause

   int cameraNum;                       /// Camera number
   int settings;                        /// Request settings from the camera
   int sensor_mode;                     /// Sensor mode. 0=auto. Check docs/forum for modes selected by other values.

   int frame;
   char *pts_filename;
   int save_pts;
   int64_t starttime;
   int64_t lasttime;

   bool netListen;
};

static XREF_T  initial_map[] =
{
   {"record",     0},
   {"pause",      1},
};

static int initial_map_size = sizeof(initial_map) / sizeof(initial_map[0]);


static void display_valid_parameters(char *app_name);

/// Command ID's and Structure defining our command line options
#define CommandHelp         0
#define CommandWidth        1
#define CommandHeight       2
#define CommandOutput       3
#define CommandVerbose      4
#define CommandTimeout      5
#define CommandDemoMode     6
#define CommandFramerate    7
#define CommandTimed        8
#define CommandSignal       9
#define CommandKeypress     10
#define CommandInitialState 11
#define CommandCamSelect    12
#define CommandSettings     13
#define CommandSensorMode   14
#define CommandOnlyLuma     15
#define CommandUseRGB       16
#define CommandSavePTS      17
#define CommandNetListen    18

static COMMAND_LIST cmdline_commands[] =
{
   { CommandHelp,          "-help",       "?",  "This help information", 0 },
   { CommandWidth,         "-width",      "w",  "Set image width <size>. Default 1920", 1 },
   { CommandHeight,        "-height",     "h",  "Set image height <size>. Default 1080", 1 },
   { CommandOutput,        "-output",     "o",  "Output filename <filename> (to write to stdout, use '-o -')", 1 },
   { CommandVerbose,       "-verbose",    "v",  "Output verbose information during run", 0 },
   { CommandTimeout,       "-timeout",    "t",  "Time (in ms) to capture for. If not specified, set to 5s. Zero to disable", 1 },
   { CommandDemoMode,      "-demo",       "d",  "Run a demo mode (cycle through range of camera options, no capture)", 1},
   { CommandFramerate,     "-framerate",  "fps","Specify the frames per second to record", 1},
   { CommandTimed,         "-timed",      "td", "Cycle between capture and pause. -cycle on,off where on is record time and off is pause time in ms", 0},
   { CommandSignal,        "-signal",     "s",  "Cycle between capture and pause on Signal", 0},
   { CommandKeypress,      "-keypress",   "k",  "Cycle between capture and pause on ENTER", 0},
   { CommandInitialState,  "-initial",    "i",  "Initial state. Use 'record' or 'pause'. Default 'record'", 1},
   { CommandCamSelect,     "-camselect",  "cs", "Select camera <number>. Default 0", 1 },
   { CommandSettings,      "-settings",   "set","Retrieve camera settings and write to stdout", 0},
   { CommandSensorMode,    "-mode",       "md", "Force sensor mode. 0=auto. See docs for other modes available", 1},
   { CommandOnlyLuma,      "-luma",       "y",  "Only output the luma / Y of the YUV data'", 0},
   { CommandUseRGB,        "-rgb",        "rgb","Save as RGB data rather than YUV", 0},
   { CommandSavePTS,       "-save-pts",   "pts","Save Timestamps to file", 1 },
   { CommandNetListen,     "-listen",     "l", "Listen on a TCP socket", 0},
};

static int cmdline_commands_size = sizeof(cmdline_commands) / sizeof(cmdline_commands[0]);


static struct
{
   char *description;
   int nextWaitMethod;
} wait_method_description[] =
{
      {"Simple capture",         WAIT_METHOD_NONE},
      {"Capture forever",        WAIT_METHOD_FOREVER},
      {"Cycle on time",          WAIT_METHOD_TIMED},
      {"Cycle on keypress",      WAIT_METHOD_KEYPRESS},
      {"Cycle on signal",        WAIT_METHOD_SIGNAL},
};

static int wait_method_description_size = sizeof(wait_method_description) / sizeof(wait_method_description[0]);



/**
 * Assign a default set of parameters to the state passed in
 *
 * @param state Pointer to state structure to assign defaults to
 */
static void default_status(RASPIVIDYUV_STATE *state)
{
   if (!state)
   {
      vcos_assert(0);
      return;
   }

   // Default everything to zero
   memset(state, 0, sizeof(RASPIVIDYUV_STATE));

   // Now set anything non-zero
   state->timeout = 5000;     // 5s delay before take image
   state->width = 1920;       // Default to 1080p
   state->height = 1080;
   state->framerate = VIDEO_FRAME_RATE_NUM;
   state->demoMode = 0;
   state->demoInterval = 250; // ms
   state->waitMethod = WAIT_METHOD_NONE;
   state->onTime = 5000;
   state->offTime = 5000;

   state->bCapturing = 0;

   state->cameraNum = -1;
   state->settings = 0;
   state->sensor_mode = 0;
   state->onlyLuma = 0;

   // Setup preview window defaults
//   raspipreview_set_defaults(&state->preview_parameters);

   // Set up the camera_parameters to default
//   raspicamcontrol_set_defaults(&state->camera_parameters);
}


/**
 * Dump image state parameters to stderr.
 *
 * @param state Pointer to state structure to assign defaults to
 */
static void dump_status(RASPIVIDYUV_STATE *state)
{
   int i, size, ystride, yheight;

   if (!state)
   {
      vcos_assert(0);
      return;
   }

   fprintf(stderr, "Width %d, Height %d, filename %s\n", state->width, state->height, state->filename);
   fprintf(stderr, "framerate %d, time delay %d\n", state->framerate, state->timeout);

   // Calculate the individual image size
   // Y stride rounded to multiple of 32. U&V stride is Y stride/2 (ie multiple of 16).
   // Y height is padded to a 16. U/V height is Y height/2 (ie multiple of 8).

   // Y plane
   ystride = ((state->width + 31) & ~31);
   yheight = ((state->height + 15) & ~15);

   size = ystride * yheight;

   // U and V plane
   size += 2 * ystride/2 * yheight/2;

   fprintf(stderr, "Sub-image size %d bytes in total.\n  Y pitch %d, Y height %d, UV pitch %d, UV Height %d\n", size, ystride, yheight, ystride/2,yheight/2);

   fprintf(stderr, "Wait method : ");
   for (i=0;i<wait_method_description_size;i++)
   {
      if (state->waitMethod == wait_method_description[i].nextWaitMethod)
         fprintf(stderr, "%s", wait_method_description[i].description);
   }
   fprintf(stderr, "\nInitial state '%s'\n", raspicli_unmap_xref(state->bCapturing, initial_map, initial_map_size));
   fprintf(stderr, "\n\n");

 //  raspipreview_dump_parameters(&state->preview_parameters);
//   raspicamcontrol_dump_parameters(&state->camera_parameters);
}

/**
 * Parse the incoming command line and put resulting parameters in to the state
 *
 * @param argc Number of arguments in command line
 * @param argv Array of pointers to strings from command line
 * @param state Pointer to state structure to assign any discovered parameters to
 * @return Non-0 if failed for some reason, 0 otherwise
 */
static int parse_cmdline(int argc, const char **argv, RASPIVIDYUV_STATE *state)
{
   // Parse the command line arguments.
   // We are looking for --<something> or -<abbreviation of something>

   int valid = 1;
   int i;

   for (i = 1; i < argc && valid; i++)
   {
      int command_id, num_parameters;

      if (!argv[i])
         continue;

      if (argv[i][0] != '-')
      {
         valid = 0;
         continue;
      }

      // Assume parameter is valid until proven otherwise
      valid = 1;

      command_id = raspicli_get_command_id(cmdline_commands, cmdline_commands_size, &argv[i][1], &num_parameters);

      // If we found a command but are missing a parameter, continue (and we will drop out of the loop)
      if (command_id != -1 && num_parameters > 0 && (i + 1 >= argc) )
         continue;

      //  We are now dealing with a command line option
      switch (command_id)
      {
      case CommandHelp:
         display_valid_parameters(basename(argv[0]));
         return -1;

      case CommandWidth: // Width > 0
         if (sscanf(argv[i + 1], "%u", &state->width) != 1)
            valid = 0;
         else
            i++;
         break;

      case CommandHeight: // Height > 0
         if (sscanf(argv[i + 1], "%u", &state->height) != 1)
            valid = 0;
         else
            i++;
         break;

      case CommandOutput:  // output filename
      {
         int len = strlen(argv[i + 1]);
         if (len)
         {
            state->filename = malloc(len + 1);
            vcos_assert(state->filename);
            if (state->filename)
               strncpy(state->filename, argv[i + 1], len+1);
            i++;
         }
         else
            valid = 0;
         break;
      }

      case CommandVerbose: // display lots of data during run
         state->verbose = 1;
         break;

      case CommandTimeout: // Time to run viewfinder/capture
      {
         if (sscanf(argv[i + 1], "%u", &state->timeout) == 1)
         {
            // Ensure that if previously selected a waitMethod we don't overwrite it
            if (state->timeout == 0 && state->waitMethod == WAIT_METHOD_NONE)
               state->waitMethod = WAIT_METHOD_FOREVER;

            i++;
         }
         else
            valid = 0;
         break;
      }

      case CommandDemoMode: // Run in demo mode - no capture
      {
         // Demo mode might have a timing parameter
         // so check if a) we have another parameter, b) its not the start of the next option
         if (i + 1 < argc  && argv[i+1][0] != '-')
         {
            if (sscanf(argv[i + 1], "%u", &state->demoInterval) == 1)
            {
               // TODO : What limits do we need for timeout?
               if (state->demoInterval == 0)
                  state->demoInterval = 250; // ms

               state->demoMode = 1;
               i++;
            }
            else
               valid = 0;
         }
         else
         {
            state->demoMode = 1;
         }

         break;
      }

      case CommandFramerate: // fps to record
      {
         if (sscanf(argv[i + 1], "%u", &state->framerate) == 1)
         {
            // TODO : What limits do we need for fps 1 - 30 - 120??
            i++;
         }
         else
            valid = 0;
         break;
      }

      case CommandTimed:
      {
         if (sscanf(argv[i + 1], "%u,%u", &state->onTime, &state->offTime) == 2)
         {
            i++;

            if (state->onTime < 1000)
               state->onTime = 1000;

            if (state->offTime < 1000)
               state->offTime = 1000;

            state->waitMethod = WAIT_METHOD_TIMED;
         }
         else
            valid = 0;
         break;
      }

      case CommandKeypress:
         state->waitMethod = WAIT_METHOD_KEYPRESS;
         break;

      case CommandSignal:
         state->waitMethod = WAIT_METHOD_SIGNAL;
         // Reenable the signal
         signal(SIGUSR1, signal_handler);
         break;

      case CommandInitialState:
      {
         state->bCapturing = raspicli_map_xref(argv[i + 1], initial_map, initial_map_size);

         if( state->bCapturing == -1)
            state->bCapturing = 0;

         i++;
         break;
      }

      case CommandCamSelect:  //Select camera input port
      {
         if (sscanf(argv[i + 1], "%u", &state->cameraNum) == 1)
         {
            i++;
         }
         else
            valid = 0;
         break;
      }

      case CommandSettings:
         state->settings = 1;
         break;

      case CommandSensorMode:
      {
         if (sscanf(argv[i + 1], "%u", &state->sensor_mode) == 1)
         {
            i++;
         }
         else
            valid = 0;
         break;
      }

      case CommandOnlyLuma:
         if (state->useRGB)
         {
            fprintf(stderr, "--luma and --rgb are mutually exclusive\n");
            valid = 0;
         }
         state->onlyLuma = 1;
         break;

      case CommandUseRGB: // display lots of data during run
         if (state->onlyLuma)
         {
            fprintf(stderr, "--luma and --rgb are mutually exclusive\n");
            valid = 0;
         }
         state->useRGB = 1;
         break;

      case CommandSavePTS:  // output filename
      {
         state->save_pts = 1;
         int len = strlen(argv[i + 1]);
         if (len)
         {
            state->pts_filename = malloc(len + 1);
            vcos_assert(state->pts_filename);
            if (state->pts_filename)
               strncpy(state->pts_filename, argv[i + 1], len+1);
            i++;
         }
         else
            valid = 0;
         break;
      }

      case CommandNetListen:
      {
         state->netListen = true;

         break;
      }

      default:
      {
         // Try parsing for any image specific parameters
         // result indicates how many parameters were used up, 0,1,2
         // but we adjust by -1 as we have used one already
    /*     const char *second_arg = (i + 1 < argc) ? argv[i + 1] : NULL;
         int parms_used = (raspicamcontrol_parse_cmdline(&state->camera_parameters, &argv[i][1], second_arg));

         // Still unused, try preview options
         if (!parms_used)
            parms_used = raspipreview_parse_cmdline(&state->preview_parameters, &argv[i][1], second_arg);


         // If no parms were used, this must be a bad parameters
         if (!parms_used)
            valid = 0;
         else
            i += parms_used - 1;

         break;*/
      }
      }
   }

   if (!valid)
   {
      fprintf(stderr, "Invalid command line option (%s)\n", argv[i-1]);
      return 1;
   }

   return 0;
}

/**
 * Display usage information for the application to stdout
 *
 * @param app_name String to display as the application name
 */
static void display_valid_parameters(char *app_name)
{
   fprintf(stdout, "Display camera output to display, and optionally saves an uncompressed YUV420 file \n\n");
   fprintf(stdout, "NOTE: High resolutions and/or frame rates may exceed the bandwidth of the system due\n");
   fprintf(stdout, "to the large amounts of data being moved to the SD card. This will result in undefined\n");
   fprintf(stdout, "results in the subsequent file.\n");
   fprintf(stdout, "The raw file produced contains all the files. Each image in the files will be of size\n");
   fprintf(stdout, "width*height*1.5, unless width and/or height are not divisible by 16. Use the image size\n");
   fprintf(stdout, "displayed during the run (in verbose mode) for an accurate value\n");

   fprintf(stdout, "The Linux split command can be used to split up the file to individual frames\n");

   fprintf(stdout, "\nusage: %s [options]\n\n", app_name);

   fprintf(stdout, "Image parameter commands\n\n");

   raspicli_display_help(cmdline_commands, cmdline_commands_size);

   fprintf(stdout, "\n");

   // Help for preview options
  // raspipreview_display_help();

   // Now display any help information from the camcontrol code
  // raspicamcontrol_display_help();

   fprintf(stdout, "\n");

   return;
}

/**
 *  buffer header callback function for camera control
 *
 *  Callback will dump buffer data to the specific file
 *
 * @param port Pointer to port from which callback originated
 * @param buffer mmal buffer header pointer
 */
static void camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
   if (buffer->cmd == MMAL_EVENT_PARAMETER_CHANGED)
   {
      MMAL_EVENT_PARAMETER_CHANGED_T *param = (MMAL_EVENT_PARAMETER_CHANGED_T *)buffer->data;
      switch (param->hdr.id) {
         case MMAL_PARAMETER_CAMERA_SETTINGS:
         {
            MMAL_PARAMETER_CAMERA_SETTINGS_T *settings = (MMAL_PARAMETER_CAMERA_SETTINGS_T*)param;
            vcos_log_error("Exposure now %u, analog gain %u/%u, digital gain %u/%u",
			settings->exposure,
                        settings->analog_gain.num, settings->analog_gain.den,
                        settings->digital_gain.num, settings->digital_gain.den);
            vcos_log_error("AWB R=%u/%u, B=%u/%u",
                        settings->awb_red_gain.num, settings->awb_red_gain.den,
                        settings->awb_blue_gain.num, settings->awb_blue_gain.den
                        );
         }
         break;
      }
   }
   else if (buffer->cmd == MMAL_EVENT_ERROR)
   {
      vcos_log_error("No data received from sensor. Check all connections, including the Sunny one on the camera board");
   }
   else
   {
      vcos_log_error("Received unexpected camera control callback event, 0x%08x", buffer->cmd);
   }

   mmal_buffer_header_release(buffer);
}


/**
 * Open a file based on the settings in state
 *
 * @param state Pointer to state
 */
static FILE *open_filename(RASPIVIDYUV_STATE *pState, char *filename)
{
   FILE *new_handle = NULL;

   if (filename)
   {
      bool bNetwork = false;
      int sfd = -1, socktype;

      if(!strncmp("tcp://", filename, 6))
      {
         bNetwork = true;
         socktype = SOCK_STREAM;
      }
      else if(!strncmp("udp://", filename, 6))
      {
         if (pState->netListen)
         {
            fprintf(stderr, "No support for listening in UDP mode\n");
            exit(131);
         }
         bNetwork = true;
         socktype = SOCK_DGRAM;
      }

      if(bNetwork)
      {
         unsigned short port;
         filename += 6;
         char *colon;
         if(NULL == (colon = strchr(filename, ':')))
         {
            fprintf(stderr, "%s is not a valid IPv4:port, use something like tcp://1.2.3.4:1234 or udp://1.2.3.4:1234\n",
                    filename);
            exit(132);
         }
         if(1 != sscanf(colon + 1, "%hu", &port))
         {
            fprintf(stderr,
                    "Port parse failed. %s is not a valid network file name, use something like tcp://1.2.3.4:1234 or udp://1.2.3.4:1234\n",
                    filename);
            exit(133);
         }
         char chTmp = *colon;
         *colon = 0;

         struct sockaddr_in saddr={};
         saddr.sin_family = AF_INET;
         saddr.sin_port = htons(port);
         if(0 == inet_aton(filename, &saddr.sin_addr))
         {
            fprintf(stderr, "inet_aton failed. %s is not a valid IPv4 address\n",
                    filename);
            exit(134);
         }
         *colon = chTmp;

         if (pState->netListen)
         {
            int sockListen = socket(AF_INET, SOCK_STREAM, 0);
            if (sockListen >= 0)
            {
               int iTmp = 1;
               setsockopt(sockListen, SOL_SOCKET, SO_REUSEADDR, &iTmp, sizeof(int));//no error handling, just go on
               if (bind(sockListen, (struct sockaddr *) &saddr, sizeof(saddr)) >= 0)
               {
                  while ((-1 == (iTmp = listen(sockListen, 0))) && (EINTR == errno))
                     ;
                  if (-1 != iTmp)
                  {
                     fprintf(stderr, "Waiting for a TCP connection on %s:%"SCNu16"...",
                             inet_ntoa(saddr.sin_addr), ntohs(saddr.sin_port));
                     struct sockaddr_in cli_addr;
                     socklen_t clilen = sizeof(cli_addr);
                     while ((-1 == (sfd = accept(sockListen, (struct sockaddr *) &cli_addr, &clilen))) && (EINTR == errno))
                        ;
                     if (sfd >= 0)
                        fprintf(stderr, "Client connected from %s:%"SCNu16"\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
                     else
                        fprintf(stderr, "Error on accept: %s\n", strerror(errno));
                  }
                  else//if (-1 != iTmp)
                  {
                     fprintf(stderr, "Error trying to listen on a socket: %s\n", strerror(errno));
                  }
               }
               else//if (bind(sockListen, (struct sockaddr *) &saddr, sizeof(saddr)) >= 0)
               {
                  fprintf(stderr, "Error on binding socket: %s\n", strerror(errno));
               }
            }
            else//if (sockListen >= 0)
            {
               fprintf(stderr, "Error creating socket: %s\n", strerror(errno));
            }

            if (sockListen >= 0)//regardless success or error
               close(sockListen);//do not listen on a given port anymore
         }
         else//if (pState->netListen)
         {
            if(0 <= (sfd = socket(AF_INET, socktype, 0)))
            {
               fprintf(stderr, "Connecting to %s:%hu...", inet_ntoa(saddr.sin_addr), port);

               int iTmp = 1;
               while ((-1 == (iTmp = connect(sfd, (struct sockaddr *) &saddr, sizeof(struct sockaddr_in)))) && (EINTR == errno))
                  ;
               if (iTmp < 0)
                  fprintf(stderr, "error: %s\n", strerror(errno));
               else
                  fprintf(stderr, "connected, sending video...\n");
            }
            else
               fprintf(stderr, "Error creating socket: %s\n", strerror(errno));
         }

         if (sfd >= 0)
            new_handle = fdopen(sfd, "w");
      }
      else
      {
         new_handle = fopen(filename, "wb");
      }
   }

   if (pState->verbose)
   {
      if (new_handle)
         fprintf(stderr, "Opening output file \"%s\"\n", filename);
      else
         fprintf(stderr, "Failed to open new file \"%s\"\n", filename);
   }

   return new_handle;
}

/**
 *  buffer header callback function for camera
 *
 *  Callback will dump buffer data to internal buffer
 *
 * @param port Pointer to port from which callback originated
 * @param buffer mmal buffer header pointer
 */
static void camera_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
   MMAL_BUFFER_HEADER_T *new_buffer;

   // We pass our file handle and other stuff in via the userdata field.

   PORT_USERDATA *pData = (PORT_USERDATA *)port->userdata;

   if (pData)
   {
      int bytes_written = 0;
      int bytes_to_write = buffer->length;
     vcos_log_error("get one frame len %d", bytes_to_write);
      if (pData->pstate->onlyLuma)
         bytes_to_write = vcos_min(buffer->length, port->format->es->video.width * port->format->es->video.height);

      vcos_assert(pData->file_handle);

      if (bytes_to_write)
      {
         mmal_buffer_header_mem_lock(buffer);
         bytes_written = fwrite(buffer->data, 1, bytes_to_write, pData->file_handle);
         mmal_buffer_header_mem_unlock(buffer);

         if (bytes_written != bytes_to_write)
         {
            vcos_log_error("Failed to write buffer data (%d from %d)- aborting", bytes_written, bytes_to_write);
            pData->abort = 1;
         }
         if (pData->pts_file_handle)
         {
            // Every buffer should be a complete frame, so no need to worry about
            // fragments or duplicated timestamps. We're also in RESET_STC mode, so
            // the time on frame 0 should always be 0 anyway, but simply copy the
            // code from raspivid.
            // MMAL_TIME_UNKNOWN should never happen, but it'll corrupt the timestamps
            // file if saved.
            if(buffer->pts != MMAL_TIME_UNKNOWN)
            {
               int64_t pts;
               if(pData->pstate->frame==0)
                  pData->pstate->starttime=buffer->pts;
               pData->lasttime=buffer->pts;
               pts = buffer->pts - pData->starttime;
               fprintf(pData->pts_file_handle,"%lld.%03lld\n", pts/1000, pts%1000);
               pData->frame++;
            }
         }
      }
   }
   else
   {
      vcos_log_error("Received a camera buffer callback with no state");
   }

   // release buffer back to the pool
   mmal_buffer_header_release(buffer);

   // and send one back to the port (if still open)
   if (port->is_enabled)
   {
      MMAL_STATUS_T status;

      new_buffer = mmal_queue_get(pData->pstate->camera_pool->queue);

      if (new_buffer)
         status = mmal_port_send_buffer(port, new_buffer);

      if (!new_buffer || status != MMAL_SUCCESS)
         vcos_log_error("Unable to return a buffer to the camera port");
   }
}

/**
 * Connect two specific ports together
 *
 * @param output_port Pointer the output port
 * @param input_port Pointer the input port
 * @param Pointer to a mmal connection pointer, reassigned if function successful
 * @return Returns a MMAL_STATUS_T giving result of operation
 *
 */
static MMAL_STATUS_T connect_ports(MMAL_PORT_T *output_port, MMAL_PORT_T *input_port, MMAL_CONNECTION_T **connection)
{
   MMAL_STATUS_T status;

   status =  mmal_connection_create(connection, output_port, input_port, MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT);

   if (status == MMAL_SUCCESS)
   {
      status =  mmal_connection_enable(*connection);
      if (status != MMAL_SUCCESS)
         mmal_connection_destroy(*connection);
   }

   return status;
}

/**
 * Checks if specified port is valid and enabled, then disables it
 *
 * @param port  Pointer the port
 *
 */
static void check_disable_port(MMAL_PORT_T *port)
{
   if (port && port->is_enabled)
      mmal_port_disable(port);
}

/**
 * Handler for sigint signals
 *
 * @param signal_number ID of incoming signal.
 *
 */
static void signal_handler(int signal_number)
{
   if (signal_number == SIGUSR1)
   {
      // Handle but ignore - prevents us dropping out if started in none-signal mode
      // and someone sends us the USR1 signal anyway
   }
   else
   {
      // Going to abort on all other signals
      vcos_log_error("Aborting program\n");
      exit(130);
   }

}

/**
 * Pause for specified time, but return early if detect an abort request
 *
 * @param state Pointer to state control struct
 * @param pause Time in ms to pause
 * @param callback Struct contain an abort flag tested for early termination
 *
 */
static int pause_and_test_abort(RASPIVIDYUV_STATE *state, int pause)
{
   int wait;

   if (!pause)
      return 0;

   // Going to check every ABORT_INTERVAL milliseconds
   for (wait = 0; wait < pause; wait+= ABORT_INTERVAL)
   {
      vcos_sleep(ABORT_INTERVAL);
      if (state->callback_data.abort)
         return 1;
   }

   return 0;
}


/**
 * Function to wait in various ways (depending on settings)
 *
 * @param state Pointer to the state data
 *
 * @return !0 if to continue, 0 if reached end of run
 */
static int wait_for_next_change(RASPIVIDYUV_STATE *state)
{
   int keep_running = 1;
   static int64_t complete_time = -1;

   // Have we actually exceeded our timeout?
   int64_t current_time =  vcos_getmicrosecs64()/1000;

   if (complete_time == -1)
      complete_time =  current_time + state->timeout;

   // if we have run out of time, flag we need to exit
   if (current_time >= complete_time && state->timeout != 0)
      keep_running = 0;

   switch (state->waitMethod)
   {
   case WAIT_METHOD_NONE:
      (void)pause_and_test_abort(state, state->timeout);
      return 0;

   case WAIT_METHOD_FOREVER:
   {
      // We never return from this. Expect a ctrl-c to exit.
      while (1)
         // Have a sleep so we don't hog the CPU.
         vcos_sleep(10000);

      return 0;
   }

   case WAIT_METHOD_TIMED:
   {
      int abort;

      if (state->bCapturing)
         abort = pause_and_test_abort(state, state->onTime);
      else
         abort = pause_and_test_abort(state, state->offTime);

      if (abort)
         return 0;
      else
         return keep_running;
   }

   case WAIT_METHOD_KEYPRESS:
   {
      char ch;

      if (state->verbose)
         fprintf(stderr, "Press Enter to %s, X then ENTER to exit\n", state->bCapturing ? "pause" : "capture");

      ch = getchar();
      if (ch == 'x' || ch == 'X')
         return 0;
      else
         return keep_running;
   }

   case WAIT_METHOD_SIGNAL:
   {
      // Need to wait for a SIGUSR1 signal
      sigset_t waitset;
      int sig;
      int result = 0;

      sigemptyset( &waitset );
      sigaddset( &waitset, SIGUSR1 );

      // We are multi threaded because we use mmal, so need to use the pthread
      // variant of procmask to block SIGUSR1 so we can wait on it.
      pthread_sigmask( SIG_BLOCK, &waitset, NULL );

      if (state->verbose)
      {
         fprintf(stderr, "Waiting for SIGUSR1 to %s\n", state->bCapturing ? "pause" : "capture");
      }

      result = sigwait( &waitset, &sig );

      if (state->verbose && result != 0)
         fprintf(stderr, "Bad signal received - error %d\n", errno);

      return keep_running;
   }

   } // switch

   return keep_running;
}

/**
 * main
 */
int main(int argc, const char **argv)
{
   // Our main data storage vessel..
   RASPIVIDYUV_STATE state = {0};
   int exit_code = EX_OK;

   MMAL_STATUS_T status = MMAL_SUCCESS;
  MMAL_PORT_T *camera_preview_port = NULL;
   MMAL_PORT_T *camera_video_port = NULL;
//   MMAL_PORT_T *camera_still_port = NULL;
//   MMAL_PORT_T *preview_input_port = NULL;

   MMAL_POOL_T *pool;

   bcm_host_init();

   // Register our application with the logging system
   vcos_log_register("VeyeRaspiVid", VCOS_LOG_CATEGORY);

   signal(SIGINT, signal_handler);

   // Disable USR1 for the moment - may be reenabled if go in to signal capture mode
   signal(SIGUSR1, SIG_IGN);

   default_status(&state);

   // Do we have any parameters
   if (argc == 1)
   {
      fprintf(stdout, "\n%s Camera App %s\n\n", basename(argv[0]), VERSION_STRING);

      display_valid_parameters(basename(argv[0]));
      exit(EX_USAGE);
   }

   // Parse the command line and put options in to our status structure
   if (parse_cmdline(argc, argv, &state))
   {
      status = -1;
      exit(EX_USAGE);
   }

   if (state.verbose)
   {
      fprintf(stderr, "\n%s Camera App %s\n\n", basename(argv[0]), VERSION_STRING);
      dump_status(&state);
   }

   // OK, we have a nice set of parameters. Now set up our components
   // We have two components. Camera, Preview
   if ((status = create_veye_camera_isp_component(&state.veye_camera_isp_state,state.cameraNum)) != MMAL_SUCCESS)
   {
      vcos_log_error("%s: Failed to create camera component", __func__);
      exit_code = EX_SOFTWARE;
   }
   else
   {
      if (state.verbose)
         fprintf(stderr, "Starting component connection stage\n");

	  camera_video_port = state.veye_camera_isp_state.isp_component->output[0];
	  
	   /* Create pool of buffer headers for the output port to consume */
	   pool = mmal_port_pool_create(camera_video_port, camera_video_port->buffer_num, camera_video_port->buffer_size);

	   if (!pool)
	   {
	      vcos_log_error("Failed to create buffer header pool for encoder output port %s", camera_video_port->name);
	   }

	   state.camera_pool = pool;

      camera_preview_port = state.veye_camera_isp_state.camera_component->output[MMAL_CAMERA_PREVIEW_PORT];
//      camera_video_port   = state.camera_component->output[MMAL_CAMERA_VIDEO_PORT];
//     camera_still_port   = state.camera_component->output[MMAL_CAMERA_CAPTURE_PORT];

//      preview_input_port  = state.preview_parameters.preview_component->input[0];
       status = connect_ports(camera_preview_port, state.veye_camera_isp_state.isp_component->input[0], &state.isp_connection);
	if (status != MMAL_SUCCESS)
	{
		vcos_log_error("Failed to create rawcam->isp connection");
		goto error;
	} 	
     
      if (status == MMAL_SUCCESS)
      {
         state.callback_data.file_handle = NULL;

         if (state.filename)
         {
            if (state.filename[0] == '-')
            {
               state.callback_data.file_handle = stdout;
            }
            else
            {
               state.callback_data.file_handle = open_filename(&state, state.filename);
            }

            if (!state.callback_data.file_handle)
            {
               // Notify user, carry on but discarding output buffers
               vcos_log_error("%s: Error opening output file: %s\nNo output file will be generated\n", __func__, state.filename);
            }
         }
         state.callback_data.pts_file_handle = NULL;

         if (state.pts_filename)
         {
            if (state.pts_filename[0] == '-')
            {
               state.callback_data.pts_file_handle = stdout;
            }
            else
            {
               state.callback_data.pts_file_handle = open_filename(&state, state.pts_filename);
               if (state.callback_data.pts_file_handle) /* save header for mkvmerge */
                  fprintf(state.callback_data.pts_file_handle, "# timecode format v2\n");
            }

            if (!state.callback_data.pts_file_handle)
            {
               // Notify user, carry on but discarding encoded output buffers
               fprintf(stderr, "Error opening output file: %s\nNo output file will be generated\n",state.pts_filename);
               state.save_pts=0;
            }
         }
         // Set up our userdata - this is passed though to the callback where we need the information.
         state.callback_data.pstate = &state;
         state.callback_data.abort = 0;

         camera_video_port->userdata = (struct MMAL_PORT_USERDATA_T *)&state.callback_data;

         if (state.verbose)
            fprintf(stderr, "Enabling camera video port\n");

         // Enable the camera video port and tell it its callback function
         status = mmal_port_enable(camera_video_port, camera_buffer_callback);

         if (status != MMAL_SUCCESS)
         {
            vcos_log_error("Failed to setup camera output");
            goto error;
         }

         if (state.demoMode)
         {
            // Run for the user specific time..
            int num_iterations = state.timeout / state.demoInterval;
            int i;

            if (state.verbose)
               fprintf(stderr, "Running in demo mode\n");

            for (i=0;state.timeout == 0 || i<num_iterations;i++)
            {
           //    raspicamcontrol_cycle_test(state.camera_component);
               vcos_sleep(state.demoInterval);
            }
         }
         else
         {
            // Only save stuff if we have a filename and it opened
            // Note we use the file handle copy in the callback, as the call back MIGHT change the file handle
            if (state.callback_data.file_handle)
            {
               int running = 1;

               // Send all the buffers to the camera video port
               {
                  int num = mmal_queue_length(state.camera_pool->queue);
                  int q;
                  for (q=0;q<num;q++)
                  {
                     MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(state.camera_pool->queue);

                     if (!buffer)
                        vcos_log_error("Unable to get a required buffer %d from pool queue", q);

                     if (mmal_port_send_buffer(camera_video_port, buffer)!= MMAL_SUCCESS)
                        vcos_log_error("Unable to send a buffer to camera video port (%d)", q);
                  }
               }

               while (running)
               {
                  // Change state

                  state.bCapturing = !state.bCapturing;

                  if (mmal_port_parameter_set_boolean(camera_video_port, MMAL_PARAMETER_CAPTURE, state.bCapturing) != MMAL_SUCCESS)
                  {
                     // How to handle?
                  }

                  if (state.verbose)
                  {
                     if (state.bCapturing)
                        fprintf(stderr, "Starting video capture\n");
                     else
                        fprintf(stderr, "Pausing video capture\n");
                  }

                  running = wait_for_next_change(&state);
               }

               if (state.verbose)
                  fprintf(stderr, "Finished capture\n");
            }
            else
            {
               if (state.timeout)
                  vcos_sleep(state.timeout);
               else
               {
                  // timeout = 0 so run forever
                  while(1)
                     vcos_sleep(ABORT_INTERVAL);
               }
            }
         }
      }
      else
      {
         mmal_status_to_int(status);
         vcos_log_error("%s: Failed to connect camera to preview", __func__);
      }

error:

      mmal_status_to_int(status);

      if (state.verbose)
         fprintf(stderr, "Closing down\n");

      // Disable all our ports that are not handled by connections
     check_disable_port(camera_video_port);
     if (state.isp_connection)
         mmal_connection_destroy(state.isp_connection);
   //   if (state.preview_parameters.wantPreview && state.preview_connection)
    //     mmal_connection_destroy(state.preview_connection);
//
 //     if (state.preview_parameters.preview_component)
   //      mmal_component_disable(state.preview_parameters.preview_component);

   //   if (state.camera_component)
 //        mmal_component_disable(state.camera_component);

      // Can now close our file. Note disabling ports may flush buffers which causes
      // problems if we have already closed the file!
      if (state.callback_data.file_handle && state.callback_data.file_handle != stdout)
         fclose(state.callback_data.file_handle);
      if (state.callback_data.pts_file_handle && state.callback_data.pts_file_handle != stdout)
         fclose(state.callback_data.pts_file_handle);

   	  if (state.veye_camera_isp_state.isp_component)
         mmal_component_disable(state.veye_camera_isp_state.isp_component);
      if (state.veye_camera_isp_state.camera_component)
         mmal_component_disable(state.veye_camera_isp_state.camera_component);

 //     raspipreview_destroy(&state.preview_parameters);
 //     destroy_splitter_component(&state);
      destroy_veye_camera_isp_component(&state.veye_camera_isp_state);
     if (state.camera_pool)
      {
         mmal_port_pool_destroy(camera_video_port, state.camera_pool);
      }

    //  raspipreview_destroy(&state.preview_parameters);
    //  destroy_camera_component(&state);

      if (state.verbose)
         fprintf(stderr, "Close down completed, all components disconnected, disabled and destroyed\n\n");
   }

   if (status != MMAL_SUCCESS)
      raspicamcontrol_check_configuration(128);

   return exit_code;
}


