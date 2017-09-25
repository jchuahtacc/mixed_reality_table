/*
Copyright (c) 2013, Broadcom Europe Ltd
Copyright (c) 2013, James Hughes
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
 * \file RaspiVid.c
 * Command line program to capture a camera video stream and encode it to file.
 * Also optionally display a preview/viewfinder of current camera input.
 *
 * \date 28th Feb 2013
 * \Author: James Hughes
 *
 * Description
 *
 * 3 components are created; camera, preview and video encoder.
 * Camera component has three ports, preview, video and stills.
 * This program connects preview and video to the preview and video
 * encoder. Using mmal we don't need to worry about buffers between these
 * components, but we do need to handle buffers from the encoder, which
 * are simply written straight to the file in the requisite buffer callback.
 *
 * If raw option is selected, a video splitter component is connected between
 * camera and preview. This allows us to set up callback for raw camera data
 * (in YUV420 or RGB format) which might be useful for further image processing.
 *
 * We use the RaspiCamControl code to handle the specific camera settings.
 * We use the RaspiPreview code to handle the (generic) preview window
 */

// We use some GNU extensions (basename)
#ifndef _GNU_SOURCE
   #define _GNU_SOURCE
#endif


#include "bcm_host.h"
#include "interface/vcos/vcos.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"
#include "interface/mmal/mmal_parameters_camera.h"

#include "RaspiCamControl.h"
#include "RaspiRenderer.h"


// Standard port setting for the camera component
#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2

// Port configuration for the splitter component
#define SPLITTER_OUTPUT_PORT 0
#define SPLITTER_PREVIEW_PORT 1

// Video format information
// 0 implies variable
#define VIDEO_FRAME_RATE_NUM 30
#define VIDEO_FRAME_RATE_DEN 1

/// Video render needs at least 2 buffers.
#define VIDEO_OUTPUT_BUFFERS_NUM 3

namespace raspivid {
    using namespace raspi_cam_control;

    // Max bitrate we allow for recording
    const int MAX_BITRATE_MJPEG = 25000000; // 25Mbits/s
    const int MAX_BITRATE_LEVEL4 = 25000000; // 25Mbits/s
    const int MAX_BITRATE_LEVEL42 = 62500000; // 62.5Mbits/s

    /// Interval at which we check for an failure abort during capture
    const int ABORT_INTERVAL = 100; // ms

    int mmal_status_to_int(MMAL_STATUS_T status);
    static void signal_handler(int signal_number);

    // Forward
    typedef struct RASPIVID_STATE_S RASPIVID_STATE;

    /** Struct used to pass information in encoder port userdata to callback
     */
    typedef struct
    {
       RASPIVID_STATE *pstate;              /// pointer to our state in case required in callback
       int abort;                           /// Set to 1 in callback if an error occurs to attempt to abort the capture
       char *cb_buff;                       /// Circular buffer
       int   cb_len;                        /// Length of buffer
       int   cb_wptr;                       /// Current write pointer
       int   cb_wrap;                       /// Has buffer wrapped at least once?
       int   cb_data;                       /// Valid bytes in buffer
#define IFRAME_BUFSIZE (60*1000)
       int   iframe_buff[IFRAME_BUFSIZE];          /// buffer of iframe pointers
       int   iframe_buff_wpos;
       int   iframe_buff_rpos;
       char  header_bytes[29];
       int  header_wptr;
       int  flush_buffers;
    } PORT_USERDATA;

    /** Possible raw output formats
     */
    typedef enum {
       RAW_OUTPUT_FMT_YUV = 1,
       RAW_OUTPUT_FMT_RGB,
       RAW_OUTPUT_FMT_GRAY,
    } RAW_OUTPUT_FMT;

    /** Structure containing all state information for the current run
     */
    struct RASPIVID_STATE_S
    {
       int timeout;                        /// Time taken before frame is grabbed and app then shuts down. Units are milliseconds
       uint32_t width;                          /// Requested width of image
       uint32_t height;                         /// requested height of image
       MMAL_FOURCC_T encoding;             /// Requested codec video encoding (MJPEG or H264)
       int bitrate;                        /// Requested bitrate
       uint32_t framerate;                      /// Requested frame rate (fps)
       uint32_t intraperiod;                    /// Intra-refresh period (key frame rate)
       uint32_t quantisationParameter;          /// Quantisation parameter - quality. Set bitrate 0 and set this for variable bitrate
       int bInlineHeaders;                  /// Insert inline headers to stream (SPS, PPS)
       char *filename;                     /// filename of output file
       int verbose;                        /// !0 if want detailed run information
       int immutableInput;                 /// Flag to specify whether encoder works in place or creates a new buffer. Result is preview can display either
                                           /// the camera output or the encoder output (with compression artifacts)
       MMAL_VIDEO_PROFILE_T profile;                        /// H264 profile to use for encoding
       MMAL_VIDEO_LEVEL_T level;                          /// H264 level to use for encoding

       RASPICAM_CAMERA_PARAMETERS camera_parameters; /// Camera setup parameters


       PORT_USERDATA callback_data;        /// Used to move data to the encoder callback

       int bCapturing;                     /// State of capture/pause
       int bCircularBuffer;                /// Whether we are writing to a circular buffer

       int inlineMotionVectors;             /// Encoder outputs inline Motion Vectors
       int raw_output;                      /// Output raw video from camera as well
       RAW_OUTPUT_FMT raw_output_fmt;       /// The raw video format
       int cameraNum;                       /// Camera number
       int settings;                        /// Request settings from the camera
       int sensor_mode;			            /// Sensor mode. 0=auto. Check docs/forum for modes selected by other values.
       MMAL_VIDEO_INTRA_REFRESH_T intra_refresh_type;              /// What intra refresh type to use. -1 to not set.
       int frame;
       int save_pts;
       int64_t starttime;
       int64_t lasttime;
       
       bool preview;

    };

    RASPIVID_STATE_S createRaspiVidDefaultState();

    class RaspiVid {
        public:
            static RaspiVid* create();
            static RaspiVid* create(RASPIVID_STATE_S state);
            static RaspiVid* getInstance();
            MMAL_STATUS_T init();
            MMAL_STATUS_T start();
            void stop();
            ~RaspiVid();

            /**
             * Assign a default set of parameters to the state passed in
             *
             * @param state Pointer to state structure to assign defaults to
             */
            void getDefaultState();

        protected:
            RASPIVID_STATE state;

            /**
             *  buffer header callback function for camera control
             *
             *  Callback will dump buffer data to the specific file
             *
             * @param port Pointer to port from which callback originated
             * @param buffer mmal buffer header pointer
             */
            static void camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

            /**
             *  buffer header callback function for encoder
             *
             *  Callback will dump buffer data to the specific file
             *
             * @param port Pointer to port from which callback originated
             * @param buffer mmal buffer header pointer
             */
            static void encoder_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

            /**
             *  buffer header callback function for splitter
             *
             *  Callback will dump buffer data to the specific file
             *
             * @param port Pointer to port from which callback originated
             * @param buffer mmal buffer header pointer
             */
            static void splitter_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);



        private:
            RaspiVid();
            static RaspiVid* singleton_;

            /**
             * Create the camera component, set up its ports
             *
             * @param state Pointer to state control struct
             *
             * @return MMAL_SUCCESS if all OK, something else otherwise
             *
             */

            MMAL_STATUS_T create_camera_component();
            /**
             * Destroy the camera component
             *
             * @param state Pointer to state control struct
             *
             */
            void destroy_camera_component();

            /**
             * Create the splitter component, set up its ports
             *
             * @param state Pointer to state control struct
             *
             * @return MMAL_SUCCESS if all OK, something else otherwise
             *
             */
            MMAL_STATUS_T create_splitter_component();

            /**
             * Destroy the splitter component
             *
             * @param state Pointer to state control struct
             *
             */
            void destroy_splitter_component();

            /**
             * Create the encoder component, set up its ports
             *
             * @param state Pointer to state control struct
             *
             * @return MMAL_SUCCESS if all OK, something else otherwise
             *
             */
            MMAL_STATUS_T create_encoder_component();

            /**
             * Destroy the encoder component
             *
             * @param state Pointer to state control struct
             *
             */
            void destroy_encoder_component();

            /**
             * Connect two specific ports together
             *
             * @param output_port Pointer the output port
             * @param input_port Pointer the input port
             * @param Pointer to a mmal connection pointer, reassigned if function successful
             * @return Returns a MMAL_STATUS_T giving result of operation
             *
             */
            MMAL_STATUS_T connect_ports(MMAL_PORT_T *output_port, MMAL_PORT_T *input_port, MMAL_CONNECTION_T **connection);

            /**
             * Checks if specified port is valid and enabled, then disables it
             *
             * @param port  Pointer the port
             *
             */
            void check_disable_port(MMAL_PORT_T *port);

            MMAL_STATUS_T create_components();
            MMAL_STATUS_T connect_components();
            MMAL_STATUS_T add_callbacks();

           
            static MMAL_COMPONENT_T *camera_component;    /// Pointer to the camera component
            static MMAL_COMPONENT_T *splitter_component;  /// Pointer to the splitter component
            static MMAL_COMPONENT_T *encoder_component;   /// Pointer to the encoder component
            static MMAL_CONNECTION_T *preview_connection; /// Pointer to the connection from camera or splitter to preview
            static MMAL_CONNECTION_T *splitter_connection;/// Pointer to the connection from camera to splitter
            static MMAL_CONNECTION_T *encoder_connection; /// Pointer to the connection from camera to encoder

            static MMAL_POOL_T *splitter_pool; /// Pointer to the pool of buffers used by splitter output port 0
            static MMAL_POOL_T *encoder_pool; /// Pointer to the pool of buffers used by encoder output port

            static MMAL_PORT_T *camera_preview_port;
            static MMAL_PORT_T *camera_video_port;
            static MMAL_PORT_T *camera_still_port;
            static MMAL_PORT_T *preview_input_port;
            static MMAL_PORT_T *encoder_input_port;
            static MMAL_PORT_T *encoder_output_port;
            static MMAL_PORT_T *splitter_input_port;
            static MMAL_PORT_T *splitter_output_port;
            static MMAL_PORT_T *splitter_preview_port;

            RaspiRenderer *preview_renderer;
    };
}
