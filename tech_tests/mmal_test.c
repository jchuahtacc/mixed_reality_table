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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <sysexits.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define VERSION_STRING "v1.3.12"

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

#include <stdbool.h>

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

// Max bitrate we allow for recording
const int MAX_BITRATE_MJPEG = 25000000; // 25Mbits/s
const int MAX_BITRATE_LEVEL4 = 25000000; // 25Mbits/s
const int MAX_BITRATE_LEVEL42 = 62500000; // 62.5Mbits/s

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
typedef struct RASPIVID_STATE_S RASPIVID_STATE;

/** Struct used to pass information in encoder port userdata to callback
 */
typedef struct
{
   FILE *file_handle;                   /// File handle to write buffer data to.
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
   int width;                          /// Requested width of image
   int height;                         /// requested height of image
   MMAL_FOURCC_T encoding;             /// Requested codec video encoding (MJPEG or H264)
   int bitrate;                        /// Requested bitrate
   int framerate;                      /// Requested frame rate (fps)
   int intraperiod;                    /// Intra-refresh period (key frame rate)
   int quantisationParameter;          /// Quantisation parameter - quality. Set bitrate 0 and set this for variable bitrate
   int bInlineHeaders;                  /// Insert inline headers to stream (SPS, PPS)
   char *filename;                     /// filename of output file
   int verbose;                        /// !0 if want detailed run information
   int demoMode;                       /// Run app in demo mode
   int demoInterval;                   /// Interval between camera settings changes
   int immutableInput;                 /// Flag to specify whether encoder works in place or creates a new buffer. Result is preview can display either
                                       /// the camera output or the encoder output (with compression artifacts)
   int profile;                        /// H264 profile to use for encoding
   int level;                          /// H264 level to use for encoding
   int waitMethod;                     /// Method for switching between pause and capture

   int onTime;                         /// In timed cycle mode, the amount of time the capture is on per cycle
   int offTime;                        /// In timed cycle mode, the amount of time the capture is off per cycle

   int segmentSize;                    /// Segment mode In timed cycle mode, the amount of time the capture is off per cycle
   int segmentWrap;                    /// Point at which to wrap segment counter
   int segmentNumber;                  /// Current segment counter
   int splitNow;                       /// Split at next possible i-frame if set to 1.
   int splitWait;                      /// Switch if user wants splited files

   RASPIPREVIEW_PARAMETERS preview_parameters;   /// Preview setup parameters
   RASPICAM_CAMERA_PARAMETERS camera_parameters; /// Camera setup parameters

   MMAL_COMPONENT_T *camera_component;    /// Pointer to the camera component
   MMAL_COMPONENT_T *splitter_component;  /// Pointer to the splitter component
   MMAL_COMPONENT_T *encoder_component;   /// Pointer to the encoder component
   MMAL_CONNECTION_T *preview_connection; /// Pointer to the connection from camera or splitter to preview
   MMAL_CONNECTION_T *splitter_connection;/// Pointer to the connection from camera to splitter
   MMAL_CONNECTION_T *encoder_connection; /// Pointer to the connection from camera to encoder

   MMAL_POOL_T *splitter_pool; /// Pointer to the pool of buffers used by splitter output port 0
   MMAL_POOL_T *encoder_pool; /// Pointer to the pool of buffers used by encoder output port

   PORT_USERDATA callback_data;        /// Used to move data to the encoder callback

   int bCapturing;                     /// State of capture/pause
   int bCircularBuffer;                /// Whether we are writing to a circular buffer

   int inlineMotionVectors;             /// Encoder outputs inline Motion Vectors
   char *imv_filename;                  /// filename of inline Motion Vectors output
   int raw_output;                      /// Output raw video from camera as well
   RAW_OUTPUT_FMT raw_output_fmt;       /// The raw video format
   char *raw_filename;                  /// Filename for raw video output
   int cameraNum;                       /// Camera number
   int settings;                        /// Request settings from the camera
   int sensor_mode;			            /// Sensor mode. 0=auto. Check docs/forum for modes selected by other values.
   int intra_refresh_type;              /// What intra refresh type to use. -1 to not set.
   int frame;
   char *pts_filename;
   int save_pts;
   int64_t starttime;
   int64_t lasttime;

   bool netListen;
};


/// Structure to cross reference H264 profile strings against the MMAL parameter equivalent
static XREF_T  profile_map[] =
{
   {"baseline",     MMAL_VIDEO_PROFILE_H264_BASELINE},
   {"main",         MMAL_VIDEO_PROFILE_H264_MAIN},
   {"high",         MMAL_VIDEO_PROFILE_H264_HIGH},
//   {"constrained",  MMAL_VIDEO_PROFILE_H264_CONSTRAINED_BASELINE} // Does anyone need this?
};

static int profile_map_size = sizeof(profile_map) / sizeof(profile_map[0]);

/// Structure to cross reference H264 level strings against the MMAL parameter equivalent
static XREF_T  level_map[] =
{
   {"4",           MMAL_VIDEO_LEVEL_H264_4},
   {"4.1",         MMAL_VIDEO_LEVEL_H264_41},
   {"4.2",         MMAL_VIDEO_LEVEL_H264_42},
};

static int level_map_size = sizeof(level_map) / sizeof(level_map[0]);


static XREF_T  initial_map[] =
{
   {"record",     0},
   {"pause",      1},
};

static int initial_map_size = sizeof(initial_map) / sizeof(initial_map[0]);

static XREF_T  intra_refresh_map[] =
{
   {"cyclic",       MMAL_VIDEO_INTRA_REFRESH_CYCLIC},
   {"adaptive",     MMAL_VIDEO_INTRA_REFRESH_ADAPTIVE},
   {"both",         MMAL_VIDEO_INTRA_REFRESH_BOTH},
   {"cyclicrows",   MMAL_VIDEO_INTRA_REFRESH_CYCLIC_MROWS},
//   {"random",       MMAL_VIDEO_INTRA_REFRESH_PSEUDO_RAND} Cannot use random, crashes the encoder. No idea why.
};

static int intra_refresh_map_size = sizeof(intra_refresh_map) / sizeof(intra_refresh_map[0]);

static XREF_T  raw_output_fmt_map[] =
{
   {"yuv",  RAW_OUTPUT_FMT_YUV},
   {"rgb",  RAW_OUTPUT_FMT_RGB},
   {"gray", RAW_OUTPUT_FMT_GRAY},
};

static int raw_output_fmt_map_size = sizeof(raw_output_fmt_map) / sizeof(raw_output_fmt_map[0]);

/// Command ID's and Structure defining our command line options
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
static void default_status(RASPIVID_STATE *state)
{
   if (!state)
   {
      vcos_assert(0);
      return;
   }

   // Default everything to zero
   memset(state, 0, sizeof(RASPIVID_STATE));

   state->raw_output_fmt = RAW_OUTPUT_FMT_GRAY;
   state->verbose = true;
   state->inlineMotionVectors = 1;

   // Now set anything non-zero
   state->timeout = 5000;     // 5s delay before take image
   state->width = 1920;       // Default to 1080p
   state->height = 1080;
   state->encoding = MMAL_ENCODING_H264;
   state->bitrate = 17000000; // This is a decent default bitrate for 1080p
   state->framerate = VIDEO_FRAME_RATE_NUM;
   state->intraperiod = -1;    // Not set
   state->quantisationParameter = 0;
   state->demoMode = 0;
   state->demoInterval = 250; // ms
   state->immutableInput = 1;
   state->profile = MMAL_VIDEO_PROFILE_H264_HIGH;
   state->level = MMAL_VIDEO_LEVEL_H264_4;
   state->waitMethod = WAIT_METHOD_NONE;
   state->onTime = 5000;
   state->offTime = 5000;

   state->bCapturing = 0;
   state->bInlineHeaders = 0;

   state->segmentSize = 0;  // 0 = not segmenting the file.
   state->segmentNumber = 1;
   state->segmentWrap = 0; // Point at which to wrap segment number back to 1. 0 = no wrap
   state->splitNow = 0;
   state->splitWait = 0;

   state->inlineMotionVectors = 0;
   state->cameraNum = 0;
   state->settings = 0;
   state->sensor_mode = 0;

   state->intra_refresh_type = -1;

   state->frame = 0;
   state->save_pts = 0;

   state->netListen = false;


   // Setup preview window defaults
   raspipreview_set_defaults(&state->preview_parameters);

   // Set up the camera_parameters to default
   raspicamcontrol_set_defaults(&state->camera_parameters);
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
 *  buffer header callback function for encoder
 *
 *  Callback will dump buffer data to the specific file
 *
 * @param port Pointer to port from which callback originated
 * @param buffer mmal buffer header pointer
 */
static void encoder_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
    MMAL_BUFFER_HEADER_T *new_buffer;
    static int64_t base_time =  -1;
    static int64_t last_second = -1;

    // All our segment times based on the receipt of the first encoder callback
    if (base_time == -1)
        base_time = vcos_getmicrosecs64()/1000;

    // We pass our file handle and other stuff in via the userdata field.
    PORT_USERDATA *pData = (PORT_USERDATA *)port->userdata;

    if (pData) {
        int bytes_written = buffer->length;
        int64_t current_time = vcos_getmicrosecs64()/1000;

        if (buffer->length) {
            mmal_buffer_header_mem_lock(buffer);
            if(buffer->flags & MMAL_BUFFER_HEADER_FLAG_CODECSIDEINFO) {
                fprintf(stderr, "Motion vector buffer length %i", buffer->length);
                //fprintf(stderr, "Saving inline Motion Vector Data\n");
            }
            mmal_buffer_header_mem_unlock(buffer);
        }

        // See if the second count has changed and we need to update any annotation
        if (current_time/1000 != last_second) {
            //  update_annotation_data(pData->pstate);
            last_second = current_time/1000;
        }
    } else {
        vcos_log_error("Received a encoder buffer callback with no state");
    }

    // release buffer back to the pool
    mmal_buffer_header_release(buffer);

    // and send one back to the port (if still open)
    if (port->is_enabled) {
      MMAL_STATUS_T status;

      new_buffer = mmal_queue_get(pData->pstate->encoder_pool->queue);

      if (new_buffer)
         status = mmal_port_send_buffer(port, new_buffer);

      if (!new_buffer || status != MMAL_SUCCESS)
         vcos_log_error("Unable to return a buffer to the encoder port");
    }
}

/**
 *  buffer header callback function for splitter
 *
 *  Callback will dump buffer data to the specific file
 *
 * @param port Pointer to port from which callback originated
 * @param buffer mmal buffer header pointer
 */
static void splitter_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
    MMAL_BUFFER_HEADER_T *new_buffer;
    PORT_USERDATA *pData = (PORT_USERDATA *)port->userdata;

    if (pData){
        int bytes_written = 0;
        int bytes_to_write = buffer->length;

        /* Write only luma component to get grayscale image: */
        if (buffer->length)
            bytes_to_write = port->format->es->video.width * port->format->es->video.height;

        if (bytes_to_write) {
            mmal_buffer_header_mem_lock(buffer);
            // DO MEMCPY HERE FOR USEFUL DATA
            // bytes_written = fwrite(buffer->data, 1, bytes_to_write, pData->raw_file_handle);
            fprintf(stderr, "Frame buffer luma channel length %i\n", bytes_to_write);
            mmal_buffer_header_mem_unlock(buffer);
        }
    } else {
      vcos_log_error("Received a camera buffer callback with no state");
    }

    // release buffer back to the pool
    mmal_buffer_header_release(buffer);

    // and send one back to the port (if still open)
    if (port->is_enabled)
    {
      MMAL_STATUS_T status;

      new_buffer = mmal_queue_get(pData->pstate->splitter_pool->queue);

      if (new_buffer)
         status = mmal_port_send_buffer(port, new_buffer);

      if (!new_buffer || status != MMAL_SUCCESS)
         vcos_log_error("Unable to return a buffer to the splitter port");
    }
}

/**
 * Create the camera component, set up its ports
 *
 * @param state Pointer to state control struct
 *
 * @return MMAL_SUCCESS if all OK, something else otherwise
 *
 */
static MMAL_STATUS_T create_camera_component(RASPIVID_STATE *state) {
    MMAL_COMPONENT_T *camera = 0;
    MMAL_ES_FORMAT_T *format;
    MMAL_PORT_T *preview_port = NULL, *video_port = NULL, *still_port = NULL;
    MMAL_STATUS_T status;

    /* Create the component */
    status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);

    if (status != MMAL_SUCCESS) {
        vcos_log_error("Failed to create camera component");
        goto error;
    }
    /*
    status = raspicamcontrol_set_stereo_mode(camera->output[0], &state->camera_parameters.stereo_mode);
    status += raspicamcontrol_set_stereo_mode(camera->output[1], &state->camera_parameters.stereo_mode);
    status += raspicamcontrol_set_stereo_mode(camera->output[2], &state->camera_parameters.stereo_mode);

    if (status != MMAL_SUCCESS)
    {
      vcos_log_error("Could not set stereo mode : error %d", status);
      goto error;
    }
    */
    MMAL_PARAMETER_INT32_T camera_num = {{MMAL_PARAMETER_CAMERA_NUM, sizeof(camera_num)}, state->cameraNum};

    status = mmal_port_parameter_set(camera->control, &camera_num.hdr);

    if (status != MMAL_SUCCESS) {
        vcos_log_error("Could not select camera : error %d", status);
        goto error;
    }

    if (!camera->output_num) {
        status = MMAL_ENOSYS;
        vcos_log_error("Camera doesn't have output ports");
        goto error;
    }

    status = mmal_port_parameter_set_uint32(camera->control, MMAL_PARAMETER_CAMERA_CUSTOM_SENSOR_CONFIG, state->sensor_mode);

    if (status != MMAL_SUCCESS) {
        vcos_log_error("Could not set sensor mode : error %d", status);
        goto error;
    }

    preview_port = camera->output[MMAL_CAMERA_PREVIEW_PORT];
    video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];
    still_port = camera->output[MMAL_CAMERA_CAPTURE_PORT];

    if (state->settings) {
        MMAL_PARAMETER_CHANGE_EVENT_REQUEST_T change_event_request =
            {{MMAL_PARAMETER_CHANGE_EVENT_REQUEST, sizeof(MMAL_PARAMETER_CHANGE_EVENT_REQUEST_T)},
            MMAL_PARAMETER_CAMERA_SETTINGS, 1};

        status = mmal_port_parameter_set(camera->control, &change_event_request.hdr);
        if ( status != MMAL_SUCCESS ) {
            vcos_log_error("No camera settings events");
        }
    }

    // Enable the camera, and tell it its control callback function
    status = mmal_port_enable(camera->control, camera_control_callback);

    if (status != MMAL_SUCCESS) {
        vcos_log_error("Unable to enable control port : error %d", status);
        goto error;
    }

    //  set up the camera configuration
    {
      MMAL_PARAMETER_CAMERA_CONFIG_T cam_config =
      {
         { MMAL_PARAMETER_CAMERA_CONFIG, sizeof(cam_config) },
         .max_stills_w = state->width,
         .max_stills_h = state->height,
         .stills_yuv422 = 0,
         .one_shot_stills = 0,
         .max_preview_video_w = state->width,
         .max_preview_video_h = state->height,
         .num_preview_video_frames = 3 + vcos_max(0, (state->framerate-30)/10),
         .stills_capture_circular_buffer_height = 0,
         .fast_preview_resume = 0,
         .use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RAW_STC
      };
      mmal_port_parameter_set(camera->control, &cam_config.hdr);
    }

    // Now set up the port formats

    // Set the encode format on the Preview port
    // HW limitations mean we need the preview to be the same size as the required recorded output

    format = preview_port->format;

    format->encoding = MMAL_ENCODING_OPAQUE;
    format->encoding_variant = MMAL_ENCODING_I420;

    if (state->camera_parameters.shutter_speed > 6000000) {
        MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)},
                                                     { 50, 1000 }, {166, 1000}};
        mmal_port_parameter_set(preview_port, &fps_range.hdr);
    } else if (state->camera_parameters.shutter_speed > 1000000) {
        MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)},
                                                     { 166, 1000 }, {999, 1000}};
        mmal_port_parameter_set(preview_port, &fps_range.hdr);
    }

    //enable dynamic framerate if necessary
    if (state->camera_parameters.shutter_speed) {
        if (state->framerate > 1000000./state->camera_parameters.shutter_speed) {
            state->framerate=0;
            if (state->verbose)
                fprintf(stderr, "Enable dynamic frame rate to fulfil shutter speed requirement\n");
        }
    }

    format->encoding = MMAL_ENCODING_OPAQUE;
    format->es->video.width = VCOS_ALIGN_UP(state->width, 32);
    format->es->video.height = VCOS_ALIGN_UP(state->height, 16);
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = state->width;
    format->es->video.crop.height = state->height;
    format->es->video.frame_rate.num = PREVIEW_FRAME_RATE_NUM;
    format->es->video.frame_rate.den = PREVIEW_FRAME_RATE_DEN;

    status = mmal_port_format_commit(preview_port);

    if (status != MMAL_SUCCESS) {
      vcos_log_error("camera viewfinder format couldn't be set");
      goto error;
    }

    // Set the encode format on the video  port

    format = video_port->format;
    format->encoding_variant = MMAL_ENCODING_I420;

    if(state->camera_parameters.shutter_speed > 6000000) {
        MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)},
                                                     { 50, 1000 }, {166, 1000}};
        mmal_port_parameter_set(video_port, &fps_range.hdr);
    } else if (state->camera_parameters.shutter_speed > 1000000) {
        MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)},
                                                     { 167, 1000 }, {999, 1000}};
        mmal_port_parameter_set(video_port, &fps_range.hdr);
    }

    format->encoding = MMAL_ENCODING_OPAQUE;
    format->es->video.width = VCOS_ALIGN_UP(state->width, 32);
    format->es->video.height = VCOS_ALIGN_UP(state->height, 16);
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = state->width;
    format->es->video.crop.height = state->height;
    format->es->video.frame_rate.num = state->framerate;
    format->es->video.frame_rate.den = VIDEO_FRAME_RATE_DEN;

    status = mmal_port_format_commit(video_port);

    if (status != MMAL_SUCCESS) {
        vcos_log_error("camera video format couldn't be set");
        goto error;
    }

    // Ensure there are enough buffers to avoid dropping frames
    if (video_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
        video_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;


    // Set the encode format on the still  port

    format = still_port->format;

    format->encoding = MMAL_ENCODING_OPAQUE;
    format->encoding_variant = MMAL_ENCODING_I420;

    format->es->video.width = VCOS_ALIGN_UP(state->width, 32);
    format->es->video.height = VCOS_ALIGN_UP(state->height, 16);
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = state->width;
    format->es->video.crop.height = state->height;
    format->es->video.frame_rate.num = 0;
    format->es->video.frame_rate.den = 1;

    status = mmal_port_format_commit(still_port);

    if (status != MMAL_SUCCESS) {
        vcos_log_error("camera still format couldn't be set");
        goto error;
    }

    /* Ensure there are enough buffers to avoid dropping frames */
    if (still_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
        still_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;

    /* Enable component */
    status = mmal_component_enable(camera);

    if (status != MMAL_SUCCESS) {
        vcos_log_error("camera component couldn't be enabled");
        goto error;
    }

    raspicamcontrol_set_all_parameters(camera, &state->camera_parameters);

    state->camera_component = camera;

    if (state->verbose)
        fprintf(stderr, "Camera component done\n");

    return status;

    error:

    if (camera)
      mmal_component_destroy(camera);

    return status;
}

/**
 * Destroy the camera component
 *
 * @param state Pointer to state control struct
 *
 */
static void destroy_camera_component(RASPIVID_STATE *state)
{
   if (state->camera_component)
   {
      mmal_component_destroy(state->camera_component);
      state->camera_component = NULL;
   }
}

/**
 * Create the splitter component, set up its ports
 *
 * @param state Pointer to state control struct
 *
 * @return MMAL_SUCCESS if all OK, something else otherwise
 *
 */
static MMAL_STATUS_T create_splitter_component(RASPIVID_STATE *state)
{
   MMAL_COMPONENT_T *splitter = 0;
   MMAL_PORT_T *splitter_output = NULL;
   MMAL_ES_FORMAT_T *format;
   MMAL_STATUS_T status;
   MMAL_POOL_T *pool;
   int i;

   if (state->camera_component == NULL)
   {
      status = MMAL_ENOSYS;
      vcos_log_error("Camera component must be created before splitter");
      goto error;
   }

   /* Create the component */
   status = mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_SPLITTER, &splitter);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("Failed to create splitter component");
      goto error;
   }

   if (!splitter->input_num)
   {
      status = MMAL_ENOSYS;
      vcos_log_error("Splitter doesn't have any input port");
      goto error;
   }

   if (splitter->output_num < 2)
   {
      status = MMAL_ENOSYS;
      vcos_log_error("Splitter doesn't have enough output ports");
      goto error;
   }

   /* Ensure there are enough buffers to avoid dropping frames: */
   mmal_format_copy(splitter->input[0]->format, state->camera_component->output[MMAL_CAMERA_PREVIEW_PORT]->format);

   if (splitter->input[0]->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
      splitter->input[0]->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;

   status = mmal_port_format_commit(splitter->input[0]);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("Unable to set format on splitter input port");
      goto error;
   }

   /* Splitter can do format conversions, configure format for its output port: */
   for (i = 0; i < splitter->output_num; i++)
   {
      mmal_format_copy(splitter->output[i]->format, splitter->input[0]->format);

      if (i == SPLITTER_OUTPUT_PORT)
      {
         format = splitter->output[i]->format;

         switch (state->raw_output_fmt)
         {
         case RAW_OUTPUT_FMT_YUV:
         case RAW_OUTPUT_FMT_GRAY: /* Grayscale image contains only luma (Y) component */
            format->encoding = MMAL_ENCODING_I420;
            format->encoding_variant = MMAL_ENCODING_I420;
            break;
         case RAW_OUTPUT_FMT_RGB:
            if (mmal_util_rgb_order_fixed(state->camera_component->output[MMAL_CAMERA_CAPTURE_PORT]))
               format->encoding = MMAL_ENCODING_RGB24;
            else
               format->encoding = MMAL_ENCODING_BGR24;
            format->encoding_variant = 0;  /* Irrelevant when not in opaque mode */
            break;
         default:
            status = MMAL_EINVAL;
            vcos_log_error("unknown raw output format");
            goto error;
         }
      }

      status = mmal_port_format_commit(splitter->output[i]);

      if (status != MMAL_SUCCESS)
      {
         vcos_log_error("Unable to set format on splitter output port %d", i);
         goto error;
      }
   }

   /* Enable component */
   status = mmal_component_enable(splitter);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("splitter component couldn't be enabled");
      goto error;
   }

   /* Create pool of buffer headers for the output port to consume */
   splitter_output = splitter->output[SPLITTER_OUTPUT_PORT];
   pool = mmal_port_pool_create(splitter_output, splitter_output->buffer_num, splitter_output->buffer_size);

   if (!pool)
   {
      vcos_log_error("Failed to create buffer header pool for splitter output port %s", splitter_output->name);
   }

   state->splitter_pool = pool;
   state->splitter_component = splitter;

   if (state->verbose)
      fprintf(stderr, "Splitter component done\n");

   return status;

error:

   if (splitter)
      mmal_component_destroy(splitter);

   return status;
}

/**
 * Destroy the splitter component
 *
 * @param state Pointer to state control struct
 *
 */
static void destroy_splitter_component(RASPIVID_STATE *state)
{
   // Get rid of any port buffers first
   if (state->splitter_pool)
   {
      mmal_port_pool_destroy(state->splitter_component->output[SPLITTER_OUTPUT_PORT], state->splitter_pool);
   }

   if (state->splitter_component)
   {
      mmal_component_destroy(state->splitter_component);
      state->splitter_component = NULL;
   }
}

/**
 * Create the encoder component, set up its ports
 *
 * @param state Pointer to state control struct
 *
 * @return MMAL_SUCCESS if all OK, something else otherwise
 *
 */
static MMAL_STATUS_T create_encoder_component(RASPIVID_STATE *state)
{
   MMAL_COMPONENT_T *encoder = 0;
   MMAL_PORT_T *encoder_input = NULL, *encoder_output = NULL;
   MMAL_STATUS_T status;
   MMAL_POOL_T *pool;

   status = mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER, &encoder);

   if (status != MMAL_SUCCESS)
   {
      
      vcos_log_error("Unable to create video encoder component");
      goto error;
   }

   if (!encoder->input_num || !encoder->output_num)
   {
      status = MMAL_ENOSYS;
      vcos_log_error("Video encoder doesn't have input/output ports");
      goto error;
   }

   encoder_input = encoder->input[0];
   encoder_output = encoder->output[0];

   // We want same format on input and output
   mmal_format_copy(encoder_output->format, encoder_input->format);

   // Only supporting H264 at the moment
   encoder_output->format->encoding = state->encoding;

   if(state->encoding == MMAL_ENCODING_H264)
   {
       fprintf(stderr, "encoding h264\n");
      if(state->level == MMAL_VIDEO_LEVEL_H264_4)
      {
         if(state->bitrate > MAX_BITRATE_LEVEL4)
         {
            fprintf(stderr, "Bitrate too high: Reducing to 25MBit/s\n");
            state->bitrate = MAX_BITRATE_LEVEL4;
         }
      }
      else
      {
         if(state->bitrate > MAX_BITRATE_LEVEL42)
         {
            fprintf(stderr, "Bitrate too high: Reducing to 62.5MBit/s\n");
            state->bitrate = MAX_BITRATE_LEVEL42;
         }
      }
   }
   else if(state->encoding == MMAL_ENCODING_MJPEG)
   {
      if(state->bitrate > MAX_BITRATE_MJPEG)
      {
         fprintf(stderr, "Bitrate too high: Reducing to 25MBit/s\n");
         state->bitrate = MAX_BITRATE_MJPEG;
      }
   }
   
   encoder_output->format->bitrate = state->bitrate;

   if (state->encoding == MMAL_ENCODING_H264) {
       fprintf(stderr, "setting buffer size to recommended\n");
      encoder_output->buffer_size = encoder_output->buffer_size_recommended;
   } else {
      encoder_output->buffer_size = 256<<10;
   }


   if (encoder_output->buffer_size < encoder_output->buffer_size_min)
      encoder_output->buffer_size = encoder_output->buffer_size_min;

   encoder_output->buffer_num = encoder_output->buffer_num_recommended;

   if (encoder_output->buffer_num < encoder_output->buffer_num_min)
      encoder_output->buffer_num = encoder_output->buffer_num_min;

   // We need to set the frame rate on output to 0, to ensure it gets
   // updated correctly from the input framerate when port connected
   encoder_output->format->es->video.frame_rate.num = 0;
   encoder_output->format->es->video.frame_rate.den = 1;

   // Commit the port changes to the output port
   status = mmal_port_format_commit(encoder_output);

   if (status != MMAL_SUCCESS) {
      vcos_log_error("Unable to set format on video encoder output port");
      goto error;
   } else {
       fprintf(stderr, "successfully set encoder output format\n");
   }

   // Set the rate control parameter
   if (0)
   {
      MMAL_PARAMETER_VIDEO_RATECONTROL_T param = {{ MMAL_PARAMETER_RATECONTROL, sizeof(param)}, MMAL_VIDEO_RATECONTROL_DEFAULT};
      status = mmal_port_parameter_set(encoder_output, &param.hdr);
      if (status != MMAL_SUCCESS)
      {
         vcos_log_error("Unable to set ratecontrol");
         goto error;
      }

   }

   if (state->encoding == MMAL_ENCODING_H264 &&
       state->intraperiod != -1)
   {
       fprintf(stderr, "setting intraperiod\n");
      MMAL_PARAMETER_UINT32_T param = {{ MMAL_PARAMETER_INTRAPERIOD, sizeof(param)}, state->intraperiod};
      status = mmal_port_parameter_set(encoder_output, &param.hdr);
      if (status != MMAL_SUCCESS)
      {
         vcos_log_error("Unable to set intraperiod");
         goto error;
      }
   }

   if (state->encoding == MMAL_ENCODING_H264 &&
       state->quantisationParameter)
   {
       fprintf(stderr, "Settign quantisation\n");
      MMAL_PARAMETER_UINT32_T param = {{ MMAL_PARAMETER_VIDEO_ENCODE_INITIAL_QUANT, sizeof(param)}, state->quantisationParameter};
      status = mmal_port_parameter_set(encoder_output, &param.hdr);
      if (status != MMAL_SUCCESS)
      {
         vcos_log_error("Unable to set initial QP");
         goto error;
      }

      MMAL_PARAMETER_UINT32_T param2 = {{ MMAL_PARAMETER_VIDEO_ENCODE_MIN_QUANT, sizeof(param)}, state->quantisationParameter};
      status = mmal_port_parameter_set(encoder_output, &param2.hdr);
      if (status != MMAL_SUCCESS)
      {
         vcos_log_error("Unable to set min QP");
         goto error;
      }

      MMAL_PARAMETER_UINT32_T param3 = {{ MMAL_PARAMETER_VIDEO_ENCODE_MAX_QUANT, sizeof(param)}, state->quantisationParameter};
      status = mmal_port_parameter_set(encoder_output, &param3.hdr);
      if (status != MMAL_SUCCESS)
      {
         vcos_log_error("Unable to set max QP");
         goto error;
      }

   }

   if (state->encoding == MMAL_ENCODING_H264)
   {
       fprintf(stderr, "setting video profile\n");
      MMAL_PARAMETER_VIDEO_PROFILE_T  param;
      param.hdr.id = MMAL_PARAMETER_PROFILE;
      param.hdr.size = sizeof(param);

      param.profile[0].profile = state->profile;

      if((VCOS_ALIGN_UP(state->width,16) >> 4) * (VCOS_ALIGN_UP(state->height,16) >> 4) * state->framerate > 245760)
      {
         if((VCOS_ALIGN_UP(state->width,16) >> 4) * (VCOS_ALIGN_UP(state->height,16) >> 4) * state->framerate <= 522240)
         {
            fprintf(stderr, "Too many macroblocks/s: Increasing H264 Level to 4.2\n");
            state->level=MMAL_VIDEO_LEVEL_H264_42;
         }
         else
         {
            vcos_log_error("Too many macroblocks/s requested");
            goto error;
         }
      }
      
      param.profile[0].level = state->level;

      status = mmal_port_parameter_set(encoder_output, &param.hdr);
      if (status != MMAL_SUCCESS)
      {
         vcos_log_error("Unable to set H264 profile");
         goto error;
      }
   }
   // REQUIRED but not sufficient... something else makes sure we get that callback
   //set INLINE VECTORS flag to request motion vector estimates
   if (mmal_port_parameter_set_boolean(encoder_output, MMAL_PARAMETER_VIDEO_ENCODE_INLINE_VECTORS, state->inlineMotionVectors) != MMAL_SUCCESS)
   {
      vcos_log_error("failed to set INLINE VECTORS parameters");
      fprintf(stderr, "Setting INLINE VECTORS parameters\n");
      // Continue rather than abort..
   } else {
       fprintf(stderr, "Not setting inline motion vector parameters!\n");
   }
   
  //  Enable component
   status = mmal_component_enable(encoder);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("Unable to enable video encoder component");
      goto error;
   }

   /* Create pool of buffer headers for the output port to consume */
   pool = mmal_port_pool_create(encoder_output, encoder_output->buffer_num, encoder_output->buffer_size);

   if (!pool)
   {
      vcos_log_error("Failed to create buffer header pool for encoder output port %s", encoder_output->name);
   }

   state->encoder_pool = pool;
   state->encoder_component = encoder;

   if (state->verbose)
      fprintf(stderr, "Encoder component done\n");

   return status;

   error:
   if (encoder)
      mmal_component_destroy(encoder);

   state->encoder_component = NULL;

   return status;
}

/**
 * Destroy the encoder component
 *
 * @param state Pointer to state control struct
 *
 */
static void destroy_encoder_component(RASPIVID_STATE *state)
{
   // Get rid of any port buffers first
   if (state->encoder_pool)
   {
      mmal_port_pool_destroy(state->encoder_component->output[0], state->encoder_pool);
   }

   if (state->encoder_component)
   {
      mmal_component_destroy(state->encoder_component);
      state->encoder_component = NULL;
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
static int pause_and_test_abort(RASPIVID_STATE *state, int pause)
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
static int wait_for_next_change(RASPIVID_STATE *state)
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
int main(int argc, const char **argv) {
    // Our main data storage vessel..
    RASPIVID_STATE state;
    int exit_code = EX_OK;

    MMAL_STATUS_T status = MMAL_SUCCESS;
    MMAL_PORT_T *camera_preview_port = NULL;
    MMAL_PORT_T *camera_video_port = NULL;
    MMAL_PORT_T *camera_still_port = NULL;
    MMAL_PORT_T *preview_input_port = NULL;
    MMAL_PORT_T *encoder_input_port = NULL;
    MMAL_PORT_T *encoder_output_port = NULL;
    MMAL_PORT_T *splitter_input_port = NULL;
    MMAL_PORT_T *splitter_output_port = NULL;
    MMAL_PORT_T *splitter_preview_port = NULL;

    bcm_host_init();

    // Register our application with the logging system
    vcos_log_register("RaspiVid", VCOS_LOG_CATEGORY);

    signal(SIGINT, signal_handler);

    // Disable USR1 for the moment - may be reenabled if go in to signal capture mode
    signal(SIGUSR1, SIG_IGN);

    default_status(&state);
    state.verbose = true;
    state.inlineMotionVectors = 1;
    state.raw_output_fmt = RAW_OUTPUT_FMT_GRAY;

    // OK, we have a nice set of parameters. Now set up our components
    // We have three components. Camera, Preview and encoder.

    if ((status = create_camera_component(&state)) != MMAL_SUCCESS) {
      vcos_log_error("%s: Failed to create camera component", __func__);
      exit_code = EX_SOFTWARE;
    }
    else if ((status = raspipreview_create(&state.preview_parameters)) != MMAL_SUCCESS) {
      vcos_log_error("%s: Failed to create preview component", __func__);
      destroy_camera_component(&state);
      exit_code = EX_SOFTWARE;
    } else if ((status = create_encoder_component(&state)) != MMAL_SUCCESS) {
      vcos_log_error("%s: Failed to create encode component", __func__);
      raspipreview_destroy(&state.preview_parameters);
      destroy_camera_component(&state);
      exit_code = EX_SOFTWARE;
    } else if (status = create_splitter_component(&state) != MMAL_SUCCESS) {
      vcos_log_error("%s: Failed to create splitter component", __func__);
      raspipreview_destroy(&state.preview_parameters);
      destroy_camera_component(&state);
      destroy_encoder_component(&state);
      exit_code = EX_SOFTWARE;
    }
    if (status != MMAL_SUCCESS) {
        fprintf(stderr, "Unable to continue without necessary component\n");
        return exit_code;
    }
    

    if (state.verbose)
        fprintf(stderr, "Starting component connection stage\n");

    camera_preview_port = state.camera_component->output[MMAL_CAMERA_PREVIEW_PORT];
    camera_video_port   = state.camera_component->output[MMAL_CAMERA_VIDEO_PORT];
    camera_still_port   = state.camera_component->output[MMAL_CAMERA_CAPTURE_PORT];
    preview_input_port  = state.preview_parameters.preview_component->input[0];
    encoder_input_port  = state.encoder_component->input[0];
    encoder_output_port = state.encoder_component->output[0];

    if (state.verbose)
        fprintf(stderr, "Raw output enabled\n");
    splitter_input_port = state.splitter_component->input[0];
    splitter_output_port = state.splitter_component->output[SPLITTER_OUTPUT_PORT];
    splitter_preview_port = state.splitter_component->output[SPLITTER_PREVIEW_PORT];

    if (state.verbose)
        fprintf(stderr, "Connecting camera preview port to splitter input port\n");

    // Connect camera to splitter
    status = connect_ports(camera_preview_port, splitter_input_port, &state.splitter_connection);
    if (status != MMAL_SUCCESS) {
       state.splitter_connection = NULL;
       vcos_log_error("%s: Failed to connect camera preview port to splitter input", __func__);
       goto error;
    }

    if (state.preview_parameters.wantPreview) {
        if (state.verbose) {
           fprintf(stderr, "Connecting splitter preview port to preview input port\n");
        }
        status = connect_ports(splitter_preview_port, preview_input_port, &state.preview_connection);

        if (status != MMAL_SUCCESS) {
            state.preview_connection = NULL;
            goto error;
        }
    } else {
        // Connect a null sink component here!
    }

    if (status == MMAL_SUCCESS) {
        if (state.verbose)
            fprintf(stderr, "Connecting camera video port to encoder input port\n");

        // Now connect the camera to the encoder
        status = connect_ports(camera_video_port, encoder_input_port, &state.encoder_connection);

        if (status != MMAL_SUCCESS) {
            state.encoder_connection = NULL;
            vcos_log_error("%s: Failed to connect camera video port to encoder input", __func__);
            goto error;
        }
    } else {
        vcos_log_error("%s: Failed to connect splitter preview output port to anything\n");
        goto error;
    }

    // Set up our userdata - this is passed though to the callback where we need the information.
    state.callback_data.pstate = &state;
    state.callback_data.abort = 0;

    splitter_output_port->userdata = (struct MMAL_PORT_USERDATA_T *)&state.callback_data;

    if (state.verbose)
       fprintf(stderr, "Enabling splitter output port\n");

    // Enable the splitter output port and tell it its callback function
    status = mmal_port_enable(splitter_output_port, splitter_buffer_callback);

    if (status != MMAL_SUCCESS)
    {
       vcos_log_error("%s: Failed to setup splitter output port", __func__);
       goto error;
    }

    // Set up our userdata - this is passed though to the callback where we need the information.
    encoder_output_port->userdata = (struct MMAL_PORT_USERDATA_T *)&state.callback_data;

    if (state.verbose)
    fprintf(stderr, "Enabling encoder output port\n");

    // Enable the encoder output port and tell it its callback function
    status = mmal_port_enable(encoder_output_port, encoder_buffer_callback);

    if (status != MMAL_SUCCESS) {
        vcos_log_error("Failed to setup encoder output");
        goto error;
    }



    if (state.verbose)
        fprintf(stderr, "Enabling buffers for callbacks\n");
    // Only encode stuff if we have a filename and it opened
    // Note we use the copy in the callback, as the call back MIGHT change the file handle
    int running = 1;

    int encoder_pool_length = mmal_queue_length(state.encoder_pool->queue);
    int q;
    for (q = 0;q < encoder_pool_length; q++) {
        MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(state.encoder_pool->queue);

        if (!buffer)
            vcos_log_error("Unable to get a required buffer %d from pool queue", q);

        if (mmal_port_send_buffer(encoder_output_port, buffer)!= MMAL_SUCCESS)
            vcos_log_error("Unable to send a buffer to encoder output port (%d)", q);
    }

    // Send all the buffers to the splitter output port
    int splitter_pool_length = mmal_queue_length(state.splitter_pool->queue);
    for (q = 0; q < splitter_pool_length; q++) {
        MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(state.splitter_pool->queue);

        if (!buffer)
            vcos_log_error("Unable to get a required buffer %d from pool queue", q);

        if (mmal_port_send_buffer(splitter_output_port, buffer)!= MMAL_SUCCESS)
            vcos_log_error("Unable to send a buffer to splitter output port (%d)", q);
    }

    int initialCapturing=state.bCapturing;
    while (running) {
        // Change state

        state.bCapturing = !state.bCapturing;

        if (mmal_port_parameter_set_boolean(camera_video_port, MMAL_PARAMETER_CAPTURE, state.bCapturing) != MMAL_SUCCESS) {
         // How to handle?
        }

        if (state.verbose) {
            if (state.bCapturing)
                fprintf(stderr, "Starting video capture\n");
            else
                fprintf(stderr, "Pausing video capture\n");
        }

        if(state.splitWait) {
            if(state.bCapturing) {
                if (mmal_port_parameter_set_boolean(encoder_output_port, MMAL_PARAMETER_VIDEO_REQUEST_I_FRAME, 1) != MMAL_SUCCESS) {
                    vcos_log_error("failed to request I-FRAME");
                }
            } else {
                if(!initialCapturing)
                   state.splitNow = 1;
            }
            initialCapturing=0;
        }
        // What does this function do?
        running = wait_for_next_change(&state);
    }

    if (state.verbose)
        fprintf(stderr, "Finished capture\n");

error:

    mmal_status_to_int(status);

    if (state.verbose)
        fprintf(stderr, "Closing down\n");

    // Disable all our ports that are not handled by connections
    check_disable_port(camera_still_port);
    check_disable_port(encoder_output_port);
    check_disable_port(splitter_output_port);

    if (state.preview_parameters.wantPreview && state.preview_connection)
        mmal_connection_destroy(state.preview_connection);

    if (state.encoder_connection)
        mmal_connection_destroy(state.encoder_connection);

    if (state.splitter_connection)
        mmal_connection_destroy(state.splitter_connection);
    if (state.encoder_component)
        mmal_component_disable(state.encoder_component);

    if (state.preview_parameters.preview_component)
        mmal_component_disable(state.preview_parameters.preview_component);

    if (state.splitter_component)
        mmal_component_disable(state.splitter_component);

    if (state.camera_component)
        mmal_component_disable(state.camera_component);

    destroy_encoder_component(&state);
    raspipreview_destroy(&state.preview_parameters);
    destroy_splitter_component(&state);
    destroy_camera_component(&state);

    if (state.verbose)
        fprintf(stderr, "Close down completed, all components disconnected, disabled and destroyed\n\n");

    return exit_code;
}
