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


#include "RaspiVid.h"

using namespace raspi_cam_control;

namespace raspivid {

    RaspiVid* RaspiVid::singleton_ = NULL;

    MMAL_COMPONENT_T* RaspiVid::camera_component = NULL;
    MMAL_COMPONENT_T* RaspiVid::splitter_component = NULL;
    MMAL_COMPONENT_T* RaspiVid::encoder_component = NULL;
    MMAL_CONNECTION_T* RaspiVid::preview_connection = NULL;
    MMAL_CONNECTION_T* RaspiVid::splitter_connection = NULL;
    MMAL_CONNECTION_T* RaspiVid::encoder_connection = NULL;
    MMAL_POOL_T* RaspiVid::splitter_pool = NULL;
    MMAL_POOL_T* RaspiVid::encoder_pool = NULL;
    MMAL_PORT_T* RaspiVid::camera_preview_port = NULL;
    MMAL_PORT_T* RaspiVid::camera_video_port = NULL;
    MMAL_PORT_T* RaspiVid::camera_still_port = NULL;
    MMAL_PORT_T* RaspiVid::preview_input_port = NULL;
    MMAL_PORT_T* RaspiVid::encoder_input_port = NULL;
    MMAL_PORT_T* RaspiVid::encoder_output_port = NULL;
    MMAL_PORT_T* RaspiVid::splitter_input_port = NULL;
    MMAL_PORT_T* RaspiVid::splitter_output_port = NULL;
    MMAL_PORT_T* RaspiVid::splitter_preview_port = NULL;

    /**
     * Assign a default set of parameters to the state passed in
     *
     * @param state Pointer to state structure to assign defaults to
     */
    RASPIVID_STATE_S createRaspiVidDefaultState() {
        RASPIVID_STATE_S state;
        // Now set anything non-zero
        state.timeout = 5000;     // 5s delay before take image
        state.width = 1920;       // Default to 1080p
        state.height = 1080;
        state.encoding = MMAL_ENCODING_H264;
        state.bitrate = 17000000; // This is a decent default bitrate for 1080p
        state.framerate = VIDEO_FRAME_RATE_NUM;
        state.intraperiod = -1;    // Not set
        state.quantisationParameter = 0;
        state.immutableInput = 1;
        state.profile = MMAL_VIDEO_PROFILE_H264_HIGH;
        state.level = MMAL_VIDEO_LEVEL_H264_4;

        state.verbose = 1;

        state.bCapturing = 0;
        state.bInlineHeaders = 0;

        state.inlineMotionVectors = 1;
        state.cameraNum = 0;
        state.settings = 0;
        state.sensor_mode = 0;

        state.intra_refresh_type = MMAL_VIDEO_INTRA_REFRESH_CYCLIC;

        state.frame = 0;
        state.save_pts = 0;
        state.raw_output_fmt = RAW_OUTPUT_FMT_GRAY;
        state.preview = false;


        // Set up the camera_parameters to default
        raspicamcontrol_set_defaults(&state.camera_parameters);
        return state;
    }

    /**
     *  buffer header callback function for camera control
     *
     *  Callback will dump buffer data to the specific file
     *
     * @param port Pointer to port from which callback originated
     * @param buffer mmal buffer header pointer
     */
    void RaspiVid::camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
        if (buffer->cmd == MMAL_EVENT_PARAMETER_CHANGED) {
            MMAL_EVENT_PARAMETER_CHANGED_T *param = (MMAL_EVENT_PARAMETER_CHANGED_T *)buffer->data;
            switch (param->hdr.id) {
                case MMAL_PARAMETER_CAMERA_SETTINGS : {
                    MMAL_PARAMETER_CAMERA_SETTINGS_T *settings = (MMAL_PARAMETER_CAMERA_SETTINGS_T*)param;
                    vcos_log_error("Exposure now %u, analog gain %u/%u, digital gain %u/%u",
                    settings->exposure,
                            settings->analog_gain.num, settings->analog_gain.den,
                            settings->digital_gain.num, settings->digital_gain.den);
                    vcos_log_error("AWB R=%u/%u, B=%u/%u",
                            settings->awb_red_gain.num, settings->awb_red_gain.den,
                            settings->awb_blue_gain.num, settings->awb_blue_gain.den);
                }
                break;
            }
        } else if (buffer->cmd == MMAL_EVENT_ERROR) {
            vcos_log_error("No data received from sensor. Check all connections, including the Sunny one on the camera board");
        } else {
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
    void RaspiVid::encoder_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
        MMAL_BUFFER_HEADER_T *new_buffer;
        static int64_t base_time =  -1;
        static int64_t last_second = -1;

        vcos_log_error("encoder_buffer_callback");

        // All our segment times based on the receipt of the first encoder callback
        if (base_time == -1) {
            base_time = vcos_getmicrosecs64() / 1000;
        }

        // We pass our file handle and other stuff in via the userdata field.

        PORT_USERDATA *pData = (PORT_USERDATA *)port->userdata;

        if (pData) {
            int bytes_written = buffer->length;
            int64_t current_time = vcos_getmicrosecs64()/1000;

            if (pData->cb_buff) {
                int space_in_buff = pData->cb_len - pData->cb_wptr;
                int copy_to_end = space_in_buff > buffer->length ? buffer->length : space_in_buff;
                int copy_to_start = buffer->length - copy_to_end;

                if (buffer->flags & MMAL_BUFFER_HEADER_FLAG_CONFIG) {
                    if(pData->header_wptr + buffer->length > sizeof(pData->header_bytes)) {
                        vcos_log_error("Error in header bytes\n");
                    } else {
                        // These are the header bytes, save them for final output
                        /*
                        mmal_buffer_header_mem_lock(buffer);
                        memcpy(pData->header_bytes + pData->header_wptr, buffer->data, buffer->length);
                        mmal_buffer_header_mem_unlock(buffer);
                        */
                        pData->header_wptr += buffer->length;
                    }
                } else if ((buffer->flags & MMAL_BUFFER_HEADER_FLAG_CODECSIDEINFO)) {
                    // Do something with the inline motion vectors...
                    vcos_log_error("inline motion vector callback 1");
                } else {
                    /*
                    vcos_log_error("encoder_buffer_callback(): Frame data");
                    static int frame_start = -1;
                    int i;

                    if (frame_start == -1) {
                        frame_start = pData->cb_wptr;
                    }

                    if (buffer->flags & MMAL_BUFFER_HEADER_FLAG_KEYFRAME) {
                        pData->iframe_buff[pData->iframe_buff_wpos] = frame_start;
                        pData->iframe_buff_wpos = (pData->iframe_buff_wpos + 1) % IFRAME_BUFSIZE;
                    }

                    if (buffer->flags & MMAL_BUFFER_HEADER_FLAG_FRAME_END) {
                        frame_start = -1;
                    }

                    // If we overtake the iframe rptr then move the rptr along
                    if((pData->iframe_buff_rpos + 1) % IFRAME_BUFSIZE != pData->iframe_buff_wpos) {
                        while((pData->cb_wptr <= pData->iframe_buff[pData->iframe_buff_rpos] &&
                                    (pData->cb_wptr + buffer->length) > pData->iframe_buff[pData->iframe_buff_rpos]) ||
                                    ((pData->cb_wptr > pData->iframe_buff[pData->iframe_buff_rpos]) &&
                                        (pData->cb_wptr + buffer->length) > (pData->iframe_buff[pData->iframe_buff_rpos] + pData->cb_len))) {
                            pData->iframe_buff_rpos = (pData->iframe_buff_rpos + 1) % IFRAME_BUFSIZE;
                        }
                    }

                    mmal_buffer_header_mem_lock(buffer);
                    // We are pushing data into a circular buffer
                    memcpy(pData->cb_buff + pData->cb_wptr, buffer->data, copy_to_end);
                    memcpy(pData->cb_buff, buffer->data + copy_to_end, copy_to_start);
                    mmal_buffer_header_mem_unlock(buffer);

                    if((pData->cb_wptr + buffer->length) > pData->cb_len) {
                        pData->cb_wrap = 1;
                    }

                    pData->cb_wptr = (pData->cb_wptr + buffer->length) % pData->cb_len;

                    for(i = pData->iframe_buff_rpos; i != pData->iframe_buff_wpos; i = (i + 1) % IFRAME_BUFSIZE) {
                        int p = pData->iframe_buff[i];
                        if (pData->cb_buff[p] != 0 || pData->cb_buff[p+1] != 0 || pData->cb_buff[p+2] != 0 || pData->cb_buff[p+3] != 1) {
                            vcos_log_error("Error in iframe list\n");
                        }
                    }
                    */
                }
            } else {
                // For segmented record mode, we need to see if we have exceeded our time/size,
                // but also since we have inline headers turned on we need to break when we get one to
                // ensure that the new stream has the header in it. If we break on an I-frame, the
                // SPS/PPS header is actually in the previous chunk.
                /*
                if ((buffer->flags & MMAL_BUFFER_HEADER_FLAG_CONFIG) &&
                    ((pData->pstate->segmentSize && current_time > base_time + pData->pstate->segmentSize) ||
                        (pData->pstate->splitWait && pData->pstate->splitNow))) {

                    base_time = current_time;

                    pData->pstate->splitNow = 0;
                    pData->pstate->segmentNumber++;

                    // Only wrap if we have a wrap point set
                    if (pData->pstate->segmentWrap && pData->pstate->segmentNumber > pData->pstate->segmentWrap) {
                        pData->pstate->segmentNumber = 1;
                    }
                }
                */
                if (buffer->length) {
                    mmal_buffer_header_mem_lock(buffer);
                    fprintf(stderr, "Got filled buffer\n");
                    mmal_buffer_header_mem_unlock(buffer);
                }
                /*
                if (buffer->length) {
                    mmal_buffer_header_mem_lock(buffer);
                    if(buffer->flags & MMAL_BUFFER_HEADER_FLAG_CODECSIDEINFO) {
                        if(pData->pstate->inlineMotionVectors) {
                            // Do Stuff with Inline Motion Vectors
                                //bytes_written = fwrite(buffer->data, 1, buffer->length, pData->imv_file_handle);
                                // if(pData->flush_buffers) fflush(pData->imv_file_handle);
                        } else {
                            //We do not want to save inlineMotionVectors...
                            bytes_written = buffer->length;
                        }
                    } else {
                        if(pData->pstate->save_pts && (buffer->flags & MMAL_BUFFER_HEADER_FLAG_FRAME_END ||
                                buffer->flags == 0 || buffer->flags & MMAL_BUFFER_HEADER_FLAG_KEYFRAME) &&
                                !(buffer->flags & MMAL_BUFFER_HEADER_FLAG_CONFIG)) {
                            if(buffer->pts != MMAL_TIME_UNKNOWN && buffer->pts != pData->pstate->lasttime) {
                                int64_t pts;
                                if (pData->pstate->frame==0) {
                                    pData->pstate->starttime=buffer->pts;
                                }
                                pData->pstate->lasttime=buffer->pts;
                                pts = buffer->pts - pData->pstate->starttime;
                                pData->pstate->frame++;
                            }
                        }
                    }

                    mmal_buffer_header_mem_unlock(buffer);

                    if (bytes_written != buffer->length) {
                        vcos_log_error("Failed to write buffer data (%d from %d)- aborting", bytes_written, buffer->length);
                        pData->abort = 1;
                    }
                }
                */
            }
        } else {
            vcos_log_error("Received a encoder buffer callback with no state");
        }

        // release buffer back to the pool
        mmal_buffer_header_release(buffer);

        // and send one back to the port (if still open)
        if (port->is_enabled) {
            MMAL_STATUS_T status;

            new_buffer = mmal_queue_get(RaspiVid::encoder_pool->queue);
            //new_buffer = mmal_queue_get(pData->pstate->encoder_pool->queue);

            if (new_buffer) {
                status = mmal_port_send_buffer(port, new_buffer);
            }

            if (!new_buffer || status != MMAL_SUCCESS) {
                vcos_log_error("Unable to return a buffer to the encoder port");
            }
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
    void RaspiVid::splitter_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
        MMAL_BUFFER_HEADER_T *new_buffer;
        PORT_USERDATA *pData = (PORT_USERDATA *)port->userdata;

        if (pData) {
            int bytes_written = 0;
            int bytes_to_write = buffer->length;

            /* Write only luma component to get grayscale image: */
            if (buffer->length && pData->pstate->raw_output_fmt == RAW_OUTPUT_FMT_GRAY) {
                bytes_to_write = port->format->es->video.width * port->format->es->video.height;
            }


            if (bytes_to_write) {
                // write data
                mmal_buffer_header_mem_lock(buffer);

                // bytes_written = fwrite(buffer->data, 1, bytes_to_write, pData->raw_file_handle);
                mmal_buffer_header_mem_unlock(buffer);
            }
        } else {
            vcos_log_error("Received a camera buffer callback with no state");
        }

        // release buffer back to the pool
        mmal_buffer_header_release(buffer);

        // and send one back to the port (if still open)
        if (port->is_enabled) {
            MMAL_STATUS_T status;

            new_buffer = mmal_queue_get(RaspiVid::splitter_pool->queue);
            //new_buffer = mmal_queue_get(pData->pstate->splitter_pool->queue);

            if (new_buffer) {
                status = mmal_port_send_buffer(port, new_buffer);
            }

            if (!new_buffer || status != MMAL_SUCCESS) {
                vcos_log_error("Unable to return a buffer to the splitter port");
            }
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
    MMAL_STATUS_T RaspiVid::create_camera_component() {
        MMAL_COMPONENT_T *camera = 0;
        MMAL_ES_FORMAT_T *format;
        MMAL_PORT_T *preview_port = NULL, *video_port = NULL, *still_port = NULL;
        MMAL_STATUS_T status;

        /* Create the component */
        status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);

        if (status != MMAL_SUCCESS) {
            vcos_log_error("Failed to create camera component");
            return status;
        }

        MMAL_PARAMETER_INT32_T camera_num =
            {{MMAL_PARAMETER_CAMERA_NUM, sizeof(camera_num)}, state.cameraNum};

        status = mmal_port_parameter_set(camera->control, &camera_num.hdr);

        if (status != MMAL_SUCCESS) {
            vcos_log_error("Could not select camera : error %d", status);
            return status;
        }

        if (!camera->output_num) {
            status = MMAL_ENOSYS;
            vcos_log_error("Camera doesn't have output ports");
            return status;
        }

        status = mmal_port_parameter_set_uint32(camera->control, MMAL_PARAMETER_CAMERA_CUSTOM_SENSOR_CONFIG, state.sensor_mode);

        if (status != MMAL_SUCCESS) {
            vcos_log_error("Could not set sensor mode : error %d", status);
            return status;
        }

        preview_port = camera->output[MMAL_CAMERA_PREVIEW_PORT];
        video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];
        still_port = camera->output[MMAL_CAMERA_CAPTURE_PORT];

        if (state.settings) {
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
          return status;
        }

        //  set up the camera configuration
        {
            MMAL_PARAMETER_CAMERA_CONFIG_T cam_config = {
                { MMAL_PARAMETER_CAMERA_CONFIG, sizeof(cam_config) },
                .max_stills_w = state.width,
                .max_stills_h = state.height,
                .stills_yuv422 = 0,
                .one_shot_stills = 0,
                .max_preview_video_w = state.width,
                .max_preview_video_h = state.height,
                .num_preview_video_frames = 3 + vcos_max(0, (state.framerate-30)/10),
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

        if (state.camera_parameters.shutter_speed > 6000000) {
            MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)}, { 50, 1000 }, {166, 1000}};
            mmal_port_parameter_set(preview_port, &fps_range.hdr);
        } else if(state.camera_parameters.shutter_speed > 1000000) {
            MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)}, { 166, 1000 }, {999, 1000}};
            mmal_port_parameter_set(preview_port, &fps_range.hdr);
        }

        //enable dynamic framerate if necessary
        if (state.camera_parameters.shutter_speed) {
            if (state.framerate > 1000000./state.camera_parameters.shutter_speed) {
                state.framerate=0;
                if (state.verbose) {
                    fprintf(stderr, "Enable dynamic frame rate to fulfil shutter speed requirement\n");
                }
            }
        }

        format->encoding = MMAL_ENCODING_OPAQUE;
        format->es->video.width = VCOS_ALIGN_UP(state.width, 32);
        format->es->video.height = VCOS_ALIGN_UP(state.height, 16);
        format->es->video.crop.x = 0;
        format->es->video.crop.y = 0;
        format->es->video.crop.width = state.width;
        format->es->video.crop.height = state.height;
        format->es->video.frame_rate.num = PREVIEW_FRAME_RATE_NUM;
        format->es->video.frame_rate.den = PREVIEW_FRAME_RATE_DEN;

        status = mmal_port_format_commit(preview_port);

        if (status != MMAL_SUCCESS) {
            vcos_log_error("camera viewfinder format couldn't be set");
            return status;
        }

        // Set the encode format on the video  port

        format = video_port->format;
        format->encoding_variant = MMAL_ENCODING_I420;

        if (state.camera_parameters.shutter_speed > 6000000) {
            MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)}, { 50, 1000 }, {166, 1000}};
            mmal_port_parameter_set(video_port, &fps_range.hdr);
        } else if(state.camera_parameters.shutter_speed > 1000000) {
            MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)}, { 167, 1000 }, {999, 1000}};
            mmal_port_parameter_set(video_port, &fps_range.hdr);
        }

        format->encoding = MMAL_ENCODING_OPAQUE;
        format->es->video.width = VCOS_ALIGN_UP(state.width, 32);
        format->es->video.height = VCOS_ALIGN_UP(state.height, 16);
        format->es->video.crop.x = 0;
        format->es->video.crop.y = 0;
        format->es->video.crop.width = state.width;
        format->es->video.crop.height = state.height;
        format->es->video.frame_rate.num = state.framerate;
        format->es->video.frame_rate.den = VIDEO_FRAME_RATE_DEN;

        status = mmal_port_format_commit(video_port);

        if (status != MMAL_SUCCESS)
        {
          vcos_log_error("camera video format couldn't be set");
          return status;
        }

        // Ensure there are enough buffers to avoid dropping frames
        if (video_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
          video_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;


        // Set the encode format on the still  port

        format = still_port->format;

        format->encoding = MMAL_ENCODING_OPAQUE;
        format->encoding_variant = MMAL_ENCODING_I420;

        format->es->video.width = VCOS_ALIGN_UP(state.width, 32);
        format->es->video.height = VCOS_ALIGN_UP(state.height, 16);
        format->es->video.crop.x = 0;
        format->es->video.crop.y = 0;
        format->es->video.crop.width = state.width;
        format->es->video.crop.height = state.height;
        format->es->video.frame_rate.num = 0;
        format->es->video.frame_rate.den = 1;

        status = mmal_port_format_commit(still_port);

        if (status != MMAL_SUCCESS)
        {
          vcos_log_error("camera still format couldn't be set");
          return status;
        }

        /* Ensure there are enough buffers to avoid dropping frames */
        if (still_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
          still_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;

        /* Enable component */
        status = mmal_component_enable(camera);

        if (status != MMAL_SUCCESS)
        {
          vcos_log_error("camera component couldn't be enabled");
          return status;
        }

        // Note: this sets lots of parameters that were not individually addressed before.
        raspicamcontrol_set_all_parameters(camera, &state.camera_parameters);

        RaspiVid::camera_component = camera;

        if (state.verbose)
          fprintf(stderr, "Camera component done\n");

        return MMAL_SUCCESS;
    }

    /**
     * Destroy the camera component
     *
     * @param state Pointer to state control struct
     *
     */
    void RaspiVid::destroy_camera_component() {
       if (RaspiVid::camera_component) {
          mmal_component_destroy(RaspiVid::camera_component);
          RaspiVid::camera_component = NULL;
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
    MMAL_STATUS_T RaspiVid::create_splitter_component() {
        MMAL_COMPONENT_T *splitter = 0;
        MMAL_PORT_T *splitter_output = NULL;
        MMAL_ES_FORMAT_T *format;
        MMAL_STATUS_T status;
        MMAL_POOL_T *pool;
        int i;

        if (camera_component == NULL) {
            status = MMAL_ENOSYS;
            vcos_log_error("Camera component must be created before splitter");
            return status;
        }

        /* Create the component */
        status = mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_SPLITTER, &splitter);

        if (status != MMAL_SUCCESS) {
            vcos_log_error("Failed to create splitter component");
            return status;
        }

        if (!splitter->input_num) {
            status = MMAL_ENOSYS;
            vcos_log_error("Splitter doesn't have any input ports");
            return status;
        }

        if (splitter->output_num < 2) {
            status = MMAL_ENOSYS;
            vcos_log_error("Splitter doesn't have enough output ports");
            return status;
        }

        /* Ensure there are enough buffers to avoid dropping frames: */
        //mmal_format_copy(splitter->input[0]->format, state.camera_component->output[MMAL_CAMERA_PREVIEW_PORT]->format);
        mmal_format_copy(splitter->input[0]->format, RaspiVid::camera_component->output[MMAL_CAMERA_PREVIEW_PORT]->format);

        if (splitter->input[0]->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM) {
            splitter->input[0]->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;
        }

        status = mmal_port_format_commit(splitter->input[0]);

        if (status != MMAL_SUCCESS) {
            vcos_log_error("Unable to set format on splitter input port");
            return status;
        }

        /* Splitter can do format conversions, configure format for its output port: */
        for (i = 0; i < splitter->output_num; i++) {
            mmal_format_copy(splitter->output[i]->format, splitter->input[0]->format);

            if (i == SPLITTER_OUTPUT_PORT) {
                format = splitter->output[i]->format;

                switch (state.raw_output_fmt) {
                    case RAW_OUTPUT_FMT_YUV:
                    case RAW_OUTPUT_FMT_GRAY: /* Grayscale image contains only luma (Y) component */
                        format->encoding = MMAL_ENCODING_I420;
                        format->encoding_variant = MMAL_ENCODING_I420;
                        break;
                    case RAW_OUTPUT_FMT_RGB:
                        //if (mmal_util_rgb_order_fixed(state.camera_component->output[MMAL_CAMERA_CAPTURE_PORT])) {
                        if (mmal_util_rgb_order_fixed(RaspiVid::camera_component->output[MMAL_CAMERA_CAPTURE_PORT])) {
                           format->encoding = MMAL_ENCODING_RGB24;
                        } else {
                           format->encoding = MMAL_ENCODING_BGR24;
                        }
                        format->encoding_variant = 0;  /* Irrelevant when not in opaque mode */
                        break;
                    default:
                        status = MMAL_EINVAL;
                        vcos_log_error("RaspiVid::create_encoder_component(): unknown raw output format");
                        return status;
                }
            }

            status = mmal_port_format_commit(splitter->output[i]);

            if (status != MMAL_SUCCESS) {
                vcos_log_error("Unable to set format on splitter output port %d", i);
                return status;
            }
        }

        /* Enable component */
        status = mmal_component_enable(splitter);

        if (status != MMAL_SUCCESS) {
            vcos_log_error("splitter component couldn't be enabled");
            return status;
        }

        /* Create pool of buffer headers for the output port to consume */
        splitter_output = splitter->output[SPLITTER_OUTPUT_PORT];
        pool = mmal_port_pool_create(splitter_output, splitter_output->buffer_num, splitter_output->buffer_size);

        if (!pool) {
            vcos_log_error("Failed to create buffer header pool for splitter output port %s", splitter_output->name);
        }

        RaspiVid::splitter_pool = pool;
        RaspiVid::splitter_component = splitter;

        if (state.verbose) {
            fprintf(stderr, "Splitter component done\n");
        }

        return MMAL_SUCCESS;
    }

    /**
     * Destroy the splitter component
     *
     * @param state Pointer to state control struct
     *
     */
    void RaspiVid::destroy_splitter_component() {
       // Get rid of any port buffers first
       if (RaspiVid::splitter_pool) {
          mmal_port_pool_destroy(RaspiVid::splitter_component->output[SPLITTER_OUTPUT_PORT], RaspiVid::splitter_pool);
       }

       if (RaspiVid::splitter_component) {
          mmal_component_destroy(RaspiVid::splitter_component);
          RaspiVid::splitter_component = NULL;
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
    MMAL_STATUS_T RaspiVid::create_encoder_component() {
        MMAL_COMPONENT_T *encoder = 0;
        MMAL_PORT_T *encoder_input = NULL, *encoder_output = NULL;
        MMAL_STATUS_T status;
        MMAL_POOL_T *pool;

        status = mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER, &encoder);

        if (status != MMAL_SUCCESS) {
            vcos_log_error("Unable to create video encoder component");
            return status;
        }

        if (!encoder->input_num || !encoder->output_num) {
            status = MMAL_ENOSYS;
            vcos_log_error("Video encoder doesn't have input/output ports");
            return status;
        }

        encoder_input = encoder->input[0];
        encoder_output = encoder->output[0];

        // We want same format on input and output
        mmal_format_copy(encoder_output->format, encoder_input->format);

        // Only supporting H264 at the moment
        encoder_output->format->encoding = state.encoding;

        if (state.encoding == MMAL_ENCODING_H264) {
            if (state.level == MMAL_VIDEO_LEVEL_H264_4) {
                if (state.bitrate > MAX_BITRATE_LEVEL4) {
                    fprintf(stderr, "Bitrate too high: Reducing to 25MBit/s\n");
                    state.bitrate = MAX_BITRATE_LEVEL4;
                }
            } else {
                if (state.bitrate > MAX_BITRATE_LEVEL42) {
                    fprintf(stderr, "Bitrate too high: Reducing to 62.5MBit/s\n");
                    state.bitrate = MAX_BITRATE_LEVEL42;
                }
            }
        } else if(state.encoding == MMAL_ENCODING_MJPEG) {
            if (state.bitrate > MAX_BITRATE_MJPEG) {
                fprintf(stderr, "Bitrate too high: Reducing to 25MBit/s\n");
                state.bitrate = MAX_BITRATE_MJPEG;
            }
        }

        encoder_output->format->bitrate = state.bitrate;

        if (state.encoding == MMAL_ENCODING_H264) {
            encoder_output->buffer_size = encoder_output->buffer_size_recommended;
        } else {
            encoder_output->buffer_size = 256<<10;
        }


        if (encoder_output->buffer_size < encoder_output->buffer_size_min) {
            encoder_output->buffer_size = encoder_output->buffer_size_min;
        }

        encoder_output->buffer_num = encoder_output->buffer_num_recommended;

        if (encoder_output->buffer_num < encoder_output->buffer_num_min) {
            encoder_output->buffer_num = encoder_output->buffer_num_min;
        }

        // We need to set the frame rate on output to 0, to ensure it gets
        // updated correctly from the input framerate when port connected
        encoder_output->format->es->video.frame_rate.num = 0;
        encoder_output->format->es->video.frame_rate.den = 1;

        // Commit the port changes to the output port
        status = mmal_port_format_commit(encoder_output);

        if (status != MMAL_SUCCESS) {
            vcos_log_error("Unable to set format on video encoder output port");
            return status;
        }

        // Set the rate control parameter
        if (0) {
            MMAL_PARAMETER_VIDEO_RATECONTROL_T param = {{ MMAL_PARAMETER_RATECONTROL, sizeof(param)}, MMAL_VIDEO_RATECONTROL_DEFAULT};
            status = mmal_port_parameter_set(encoder_output, &param.hdr);
            if (status != MMAL_SUCCESS) {
                vcos_log_error("Unable to set ratecontrol");
                return status;
            }
        }

        /*
        if (state.encoding == MMAL_ENCODING_H264 && state.intraperiod != -1) {
            MMAL_PARAMETER_UINT32_T param = {{ MMAL_PARAMETER_INTRAPERIOD, sizeof(param)}, state.intraperiod};
            status = mmal_port_parameter_set(encoder_output, &param.hdr);
            if (status != MMAL_SUCCESS) {
             vcos_log_error("Unable to set intraperiod");
             return status;
            }
        }
        */

        if (state.encoding == MMAL_ENCODING_H264 && state.quantisationParameter) {
            MMAL_PARAMETER_UINT32_T param = {{ MMAL_PARAMETER_VIDEO_ENCODE_INITIAL_QUANT, sizeof(param)}, state.quantisationParameter};
            status = mmal_port_parameter_set(encoder_output, &param.hdr);
            if (status != MMAL_SUCCESS) {
                vcos_log_error("Unable to set initial QP");
                return status;
            }

            MMAL_PARAMETER_UINT32_T param2 = {{ MMAL_PARAMETER_VIDEO_ENCODE_MIN_QUANT, sizeof(param)}, state.quantisationParameter};
            status = mmal_port_parameter_set(encoder_output, &param2.hdr);
            if (status != MMAL_SUCCESS) {
                vcos_log_error("Unable to set min QP");
                return status;
            }

            MMAL_PARAMETER_UINT32_T param3 = {{ MMAL_PARAMETER_VIDEO_ENCODE_MAX_QUANT, sizeof(param)}, state.quantisationParameter};
            status = mmal_port_parameter_set(encoder_output, &param3.hdr);
            if (status != MMAL_SUCCESS) {
                vcos_log_error("Unable to set max QP");
                return status;
            }
        }

        if (state.encoding == MMAL_ENCODING_H264) {
            MMAL_PARAMETER_VIDEO_PROFILE_T  param;
            param.hdr.id = MMAL_PARAMETER_PROFILE;
            param.hdr.size = sizeof(param);

            param.profile[0].profile = state.profile;

            if ((VCOS_ALIGN_UP(state.width,16) >> 4) * (VCOS_ALIGN_UP(state.height,16) >> 4) * state.framerate > 245760) {
                if((VCOS_ALIGN_UP(state.width,16) >> 4) * (VCOS_ALIGN_UP(state.height,16) >> 4) * state.framerate <= 522240) {
                    fprintf(stderr, "Too many macroblocks/s: Increasing H264 Level to 4.2\n");
                    state.level = MMAL_VIDEO_LEVEL_H264_42;
                } else {
                    vcos_log_error("Too many macroblocks/s requested");
                    return status;
                }
            }

            param.profile[0].level = state.level;

            status = mmal_port_parameter_set(encoder_output, &param.hdr);
            if (status != MMAL_SUCCESS) {
                vcos_log_error("Unable to set H264 profile");
                return status;
            }
        }

        if (mmal_port_parameter_set_boolean(encoder_input, MMAL_PARAMETER_VIDEO_IMMUTABLE_INPUT, state.immutableInput) != MMAL_SUCCESS) {
            vcos_log_error("Unable to set immutable input flag");
            // Continue rather than abort..
        }

        //set INLINE HEADER flag to generate SPS and PPS for every IDR if requested
        if (mmal_port_parameter_set_boolean(encoder_output, MMAL_PARAMETER_VIDEO_ENCODE_INLINE_HEADER, state.bInlineHeaders) != MMAL_SUCCESS) {
            vcos_log_error("failed to set INLINE HEADER FLAG parameters");
            // Continue rather than abort..
        }

        //set INLINE VECTORS flag to request motion vector estimates
        if (state.encoding == MMAL_ENCODING_H264 && 
                mmal_port_parameter_set_boolean(encoder_output, MMAL_PARAMETER_VIDEO_ENCODE_INLINE_VECTORS, state.inlineMotionVectors) != MMAL_SUCCESS) {
            vcos_log_error("failed to set INLINE VECTORS parameters");
            // Continue rather than abort..
        }

        // Adaptive intra refresh settings
        if (state.encoding == MMAL_ENCODING_H264 && state.intra_refresh_type != -1) {
            MMAL_PARAMETER_VIDEO_INTRA_REFRESH_T  param;
            param.hdr.id = MMAL_PARAMETER_VIDEO_INTRA_REFRESH;
            param.hdr.size = sizeof(param);

            // Get first so we don't overwrite anything unexpectedly
            status = mmal_port_parameter_get(encoder_output, &param.hdr);
            if (status != MMAL_SUCCESS) {
                vcos_log_warn("Unable to get existing H264 intra-refresh values. Please update your firmware");
                // Set some defaults, don't just pass random stack data
                param.air_mbs = param.air_ref = param.cir_mbs = param.pir_mbs = 0;
            }

            param.refresh_mode = state.intra_refresh_type;

            //if (state->intra_refresh_type == MMAL_VIDEO_INTRA_REFRESH_CYCLIC_MROWS)
            //   param.cir_mbs = 10;

            status = mmal_port_parameter_set(encoder_output, &param.hdr);
            if (status != MMAL_SUCCESS) {
             vcos_log_error("Unable to set H264 intra-refresh values");
             return status;
            }
        }

        //  Enable component
        status = mmal_component_enable(encoder);

        if (status != MMAL_SUCCESS) {
            vcos_log_error("Unable to enable video encoder component");
            return status;
        }

        /* Create pool of buffer headers for the output port to consume */
        pool = mmal_port_pool_create(encoder_output, encoder_output->buffer_num, encoder_output->buffer_size);

        if (!pool) {
            vcos_log_error("Failed to create buffer header pool for encoder output port %s", encoder_output->name);
        }

        RaspiVid::encoder_pool = pool;
        RaspiVid::encoder_component = encoder;

        if (state.verbose) {
            fprintf(stderr, "Encoder component done\n");
        }

        return MMAL_SUCCESS;
    }

    /**
     * Destroy the encoder component
     *
     * @param state Pointer to state control struct
     *
     */
    void RaspiVid::destroy_encoder_component() {
       // Get rid of any port buffers first
       if (RaspiVid::encoder_pool) {
          mmal_port_pool_destroy(RaspiVid::encoder_component->output[0], RaspiVid::encoder_pool);
       }

       if (RaspiVid::encoder_component) {
          mmal_component_destroy(RaspiVid::encoder_component);
          RaspiVid::encoder_component = NULL;
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
    MMAL_STATUS_T RaspiVid::connect_ports(MMAL_PORT_T *output_port, MMAL_PORT_T *input_port, MMAL_CONNECTION_T **connection) {
        MMAL_STATUS_T status;

        status =  mmal_connection_create(connection, output_port, input_port, MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT);
        if (status != MMAL_SUCCESS) {
           vcos_log_error("RaspiVid::connect_ports(): unable to connect ports");
           return status;
        }
        status =  mmal_connection_enable(*connection);
        if (status != MMAL_SUCCESS) {
            vcos_log_error("RaspiVid::connect_ports(): unable to enable connection");
            mmal_connection_destroy(*connection);
            return status;
        }

        return MMAL_SUCCESS;
    }

    /**
     * Checks if specified port is valid and enabled, then disables it
     *
     * @param port  Pointer the port
     *
     */
    void RaspiVid::check_disable_port(MMAL_PORT_T *port) {
        if (port && port->is_enabled) {
            mmal_port_disable(port);
        }
    }

    MMAL_STATUS_T RaspiVid::create_components() {
        MMAL_STATUS_T status = MMAL_SUCCESS;
        if ((status = create_camera_component()) != MMAL_SUCCESS) {
          vcos_log_error("%s: Failed to create camera component", __func__);
          return status;
        }

        if (state.preview) {
            preview_renderer = RaspiRenderer::create();
            if (preview_renderer == NULL) {
                vcos_log_error("%s: Failed to create preview component", __func__);
                return MMAL_ENOSYS;
            }
            // RaspiVid::preview_input_port = RaspiVid::preview_renderer->input;
        }

        /* RASPIENCODER
        if ((status = create_encoder_component()) != MMAL_SUCCESS)
        {
          vcos_log_error("%s: Failed to create encode component", __func__);
          return status;
        }
        */

        resizer = RaspiResize::create();
        if (resizer == NULL) {
            vcos_log_error("%s: Failed to create resizer component", __func__);
            return MMAL_ENOSYS;
        }

        encoder = RaspiEncoder::create();
        if (encoder == NULL) {
            vcos_log_error("%s: Failed to create encoder component", __func__);
            return MMAL_ENOSYS;
        }

        splitter = RaspiSplitter::create();
        if (splitter == NULL) {
            vcos_log_error("%s: Failed to create splitter component", __func__);
            return MMAL_ENOSYS;
        }

        /* RASPISPLITTER

        if (state.raw_output && (status = create_splitter_component()) != MMAL_SUCCESS)
        {
          vcos_log_error("%s: Failed to create splitter component", __func__);
          return status;
        }
        */
        RaspiVid::camera_preview_port = RaspiVid::camera_component->output[MMAL_CAMERA_PREVIEW_PORT];
        RaspiVid::camera_video_port   = RaspiVid::camera_component->output[MMAL_CAMERA_VIDEO_PORT];
        RaspiVid::camera_still_port   = RaspiVid::camera_component->output[MMAL_CAMERA_CAPTURE_PORT];
        /* RASPIENCODER
        RaspiVid::encoder_input_port  = RaspiVid::encoder_component->input[0];
        RaspiVid::encoder_output_port = RaspiVid::encoder_component->output[0];
        */
        
        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RaspiVid::connect_components() {
        MMAL_STATUS_T status;
        if (state.raw_output) {
            /* RASPISPLITTER
            RaspiVid::splitter_input_port = RaspiVid::splitter_component->input[0];
            RaspiVid::splitter_output_port = RaspiVid::splitter_component->output[SPLITTER_OUTPUT_PORT];
            RaspiVid::splitter_preview_port = RaspiVid::splitter_component->output[SPLITTER_PREVIEW_PORT];
            */
        }

        if (state.preview) {
            if (state.raw_output) {
                /* RASPISPLITTER
                // Want preview, also want raw output
                // camera_preview to splitter_input
                // splitter_preview to preview_input
                if (state.verbose) { fprintf(stderr, "Connecting camera preview port to splitter input port\n"); }

                // Connect camera to splitter
                status = connect_ports(RaspiVid::camera_preview_port, RaspiVid::splitter_input_port, &RaspiVid::splitter_connection);
                if (status != MMAL_SUCCESS) {
                    RaspiVid::splitter_connection = NULL;
                    vcos_log_error("%s: Failed to connect camera preview port to splitter input", __func__);
                    return status;
                }

                if (state.verbose) {
                   fprintf(stderr, "Connecting splitter preview port to preview input port\n");
                   fprintf(stderr, "Starting video preview\n");
                }
                
                */
                vcos_assert(splitter);
                status = splitter->input->connect(RaspiVid::camera_preview_port, &RaspiVid::splitter_connection);
                if (status != MMAL_SUCCESS) {
                    RaspiVid::splitter_connection = NULL;
                    vcos_log_error("RaspiVid::connect_components(): failed to connect camera preview to splitter input");
                    return status;
                }

                if (splitter->duplicate_input() != MMAL_SUCCESS) {
                    vcos_log_error("RaspiVid::connect_components(): unable to copy splitter input format to outputs");
                }
                
                // Connect splitter to preview
                status = preview_renderer->input->connect(splitter->output_1);
                //status = preview_renderer->input->connect(RaspiVid::splitter_preview_port, &RaspiVid::preview_connection);
                if (status != MMAL_SUCCESS) {
                    vcos_log_error("failed to connect preview_renderer");
                    return status;
                }
                /*
                status = connect_ports(RaspiVid::splitter_preview_port, RaspiVid::preview_input_port, &RaspiVid::preview_connection);
                if (status != MMAL_SUCCESS) {
                    RaspiVid::preview_connection = NULL;
                    vcos_log_error("%s: Failed to connect splitter preview to preview input port", __func__);
                    return status;
                }
                */
            } else {
                // Want preview, don't want raw output
                // camera_preview to preview
                if (state.verbose) {
                   fprintf(stderr, "Connecting camera preview port to preview input port\n");
                   fprintf(stderr, "Starting video preview\n");
                }

                // Connect camera to preview
                //status = connect_ports(RaspiVid::camera_preview_port, RaspiVid::preview_input_port, &RaspiVid::preview_connection);
                status = preview_renderer->input->connect(RaspiVid::camera_preview_port, &RaspiVid::preview_connection);
                if (status != MMAL_SUCCESS) {
                    RaspiVid::preview_connection = NULL;
                    vcos_log_error("%s: Failed to connect camera preview to preview input port", __func__);
                    return status;
                }
            }
        } else {
            if (state.raw_output) {
                // Don't want preview, but want raw output
                if (state.verbose) {
                    fprintf(stderr, "Connecting camera preview port to splitter input port\n");
                }

                // Connect camera to splitter
                //status = connect_ports(RaspiVid::camera_preview_port, RaspiVid::splitter_input_port, &RaspiVid::splitter_connection);
                status = splitter->input->connect(RaspiVid::camera_preview_port, &RaspiVid::splitter_connection);
                if (status != MMAL_SUCCESS) {
                    RaspiVid::splitter_connection = NULL;
                   vcos_log_error("%s: Failed to connect camera preview port to splitter input", __func__);
                   return status;
                }
                if (splitter->duplicate_input() != MMAL_SUCCESS) {
                    vcos_log_error("RaspiComponent::connect_components(): unable to copy splitter input format to outputs");
                }

                // TODO: Connect Nullsink
            } else {
                // Don't want preview or raw output
                // TODO: Connect null sink
            }
        }

        if (state.verbose)
            fprintf(stderr, "Connecting camera video port to resizer input port\n");

        /* RASPIENCODER
        // Now connect the camera to the encoder
        status = connect_ports(RaspiVid::camera_video_port, RaspiVid::encoder_input_port, &RaspiVid::encoder_connection);

        if (status != MMAL_SUCCESS) {
            RaspiVid::encoder_connection = NULL;
            vcos_log_error("%s: Failed to connect camera video port to encoder input", __func__);
            return status;
        }
        */

        // Connect camera to resizer
        vcos_assert(resizer->input);
        vcos_assert(RaspiVid::camera_video_port);
        status = resizer->input->connect(RaspiVid::camera_video_port, &RaspiVid::encoder_connection);
        if (status != MMAL_SUCCESS) {
            vcos_log_error("Failed to connect RaspiResizer to camera_video_port");
            return status;
        }
        status = resizer->set_output(800, 600);
        if (status != MMAL_SUCCESS) {
            vcos_log_error("Failed to set output format on resizer");
            return status;
        }
        
        // Connect resizer to encoder
        vcos_assert(encoder->input);
        vcos_assert(resizer->output);
        status = encoder->input->connect(resizer->output);
        if (status != MMAL_SUCCESS) {
            vcos_log_error("Failed to connect RaspiEncoder to RaspiResizer");
            return status;
        }

        /*
        vcos_assert(encoder->input);
        vcos_assert(RaspiVid::camera_video_port);
        status = encoder->input->connect(RaspiVid::camera_video_port, &RaspiVid::encoder_connection);
        if (status != MMAL_SUCCESS) {
            vcos_log_error("Failed to connect RaspiEncoder");
            return status;
        }
        */

        return MMAL_SUCCESS;
    }


    MMAL_STATUS_T RaspiVid::add_callbacks() {
        MMAL_STATUS_T status = MMAL_SUCCESS;
        // Set up our userdata - this is passed though to the callback where we need the information.
        state.callback_data.pstate = &state;
        state.callback_data.abort = 0;

        if (state.raw_output) {
            roCallback = new RawOutputCallback();
            if (splitter->output_0->add_callback(roCallback) != MMAL_SUCCESS) {
                vcos_log_error("RaspiVid::add_callbacks(): Could not add raw output callback");
            }
            /* RASPISPLITTER
            // Want raw output - connect splitter_buffer_callback (with userdata)
            splitter_output_port->userdata = (struct MMAL_PORT_USERDATA_T *)&state.callback_data;

            if (state.verbose) {
                fprintf(stderr, "Enabling splitter output port\n");
            }

            // Enable the splitter output port and tell it its callback function
            status = mmal_port_enable(RaspiVid::splitter_output_port, RaspiVid::splitter_buffer_callback);
            if (status != MMAL_SUCCESS) {
               vcos_log_error("%s: Failed to setup splitter_buffer_callback on splitter output port", __func__);
               return status;
            }
            */

        }

        if (state.bCircularBuffer) {
            if (state.bitrate == 0) {
               vcos_log_error("%s: Error circular buffer requires constant bitrate and small intra period\n", __func__);
               return MMAL_EINVAL;
            } else if(state.timeout == 0) {
               vcos_log_error("%s: Error, circular buffer size is based on timeout must be greater than zero\n", __func__);
               return MMAL_EINVAL;
            } else {
               int count = state.bitrate * (state.timeout / 1000) / 8;

               state.callback_data.cb_buff = (char *) malloc(count);
               if (state.callback_data.cb_buff == NULL) {
                  vcos_log_error("%s: Unable to allocate circular buffer for %d seconds at %.1f Mbits\n", __func__, state.timeout / 1000, (double)state.bitrate/1000000.0);
                  return MMAL_EINVAL;
               } else {
                  state.callback_data.cb_len = count;
                  state.callback_data.cb_wptr = 0;
                  state.callback_data.cb_wrap = 0;
                  state.callback_data.cb_data = 0;
                  state.callback_data.iframe_buff_wpos = 0;
                  state.callback_data.iframe_buff_rpos = 0;
                  state.callback_data.header_wptr = 0;
               }
            }
        }

        mvCallback = new MotionVectorCallback();
        encoder->output->add_callback(mvCallback);
        /* RASPIENCODER
        // Set up our userdata - this is passed though to the callback where we need the information.
        encoder_output_port->userdata = (struct MMAL_PORT_USERDATA_T *)&state.callback_data;

        if (state.verbose) {
            fprintf(stderr, "Enabling encoder output port\n");
        }

        // Enable the encoder output port and tell it its callback function
        status = mmal_port_enable(RaspiVid::encoder_output_port, RaspiVid::encoder_buffer_callback);

        if (status != MMAL_SUCCESS) {
            vcos_log_error("Failed to setup encoder_buffer_callback on encoder output port");
            return status;
        }
        */

        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RaspiVid::init() {
        MMAL_STATUS_T status = MMAL_SUCCESS;
        bcm_host_init();

        // Register our application with the logging system
        vcos_log_register("RaspiVid", VCOS_LOG_CATEGORY);

        // OK, we have a nice set of parameters. Now set up our components
        // We have three components. Camera, Preview and encoder.

        if ((status = create_components()) != MMAL_SUCCESS) {
            vcos_log_error("RaspiVid::init(): Failed to create one or more components");
            return status;
        }

        if (state.verbose) {
            fprintf(stderr, "Starting component connection stage\n");
        }

        if ((status = connect_components()) != MMAL_SUCCESS) {
            vcos_log_error("RaspiVid::init(): Failed to connect one or more components");
            return status;
        }

        if (state.verbose) {
            fprintf(stderr, "Starting callback addition stage\n");
        }

        if ((status = add_callbacks()) != MMAL_SUCCESS) {
            vcos_log_error("RaspiVid::init(): Failed to add one or more callbacks");
            return status;

        }
        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RaspiVid::start() {
        /* RASPIENCODER
       // Send all the buffers to the encoder output port
       {
          int num = mmal_queue_length(RaspiVid::encoder_pool->queue);
          int q;
          for (q=0;q<num;q++)
          {
             MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(RaspiVid::encoder_pool->queue);

             if (!buffer)
                vcos_log_error("Unable to get a required buffer %d from pool queue", q);

             if (mmal_port_send_buffer(RaspiVid::encoder_output_port, buffer)!= MMAL_SUCCESS)
                vcos_log_error("Unable to send a buffer to encoder output port (%d)", q);
          }
       }
       */

       // Send all the buffers to the splitter output port
       /* RASPISPLITTER
       if (state.raw_output) {
          int num = mmal_queue_length(RaspiVid::splitter_pool->queue);
          for (int q = 0; q < num; q++) {
             MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(RaspiVid::splitter_pool->queue);

             if (!buffer)
                vcos_log_error("Unable to get a required buffer %d from pool queue", q);

             if (mmal_port_send_buffer(RaspiVid::splitter_output_port, buffer)!= MMAL_SUCCESS)
                vcos_log_error("Unable to send a buffer to splitter output port (%d)", q);
          }
       }
       */

       mmal_port_parameter_set_boolean(RaspiVid::camera_video_port, MMAL_PARAMETER_CAPTURE, 1);

       return MMAL_SUCCESS;
    }

    void RaspiVid::stop() {

        // Disable all our ports that are not handled by connections
        check_disable_port(RaspiVid::camera_still_port);
        check_disable_port(RaspiVid::encoder_output_port);
        check_disable_port(RaspiVid::splitter_output_port);

        if (RaspiVid::preview_connection)
            mmal_connection_destroy(RaspiVid::preview_connection);

        if (RaspiVid::encoder_connection)
            mmal_connection_destroy(RaspiVid::encoder_connection);

        if (RaspiVid::splitter_connection)
            mmal_connection_destroy(RaspiVid::splitter_connection);

        /* Disable components */
        if (RaspiVid::encoder_component) {
            mmal_component_disable(RaspiVid::encoder_component);
        }

        if (preview_renderer) {
            preview_renderer->destroy();
            delete preview_renderer;
        }

        if (RaspiVid::splitter_component) {
            mmal_component_disable(RaspiVid::splitter_component);
        }

        if (RaspiVid::camera_component) {
            mmal_component_disable(RaspiVid::camera_component);
        }

        destroy_encoder_component();
        destroy_splitter_component();
        destroy_camera_component();

        if (mvCallback) {
            delete mvCallback;
        }

        if (roCallback) {
            delete roCallback;
        }

        if (state.verbose) {
            fprintf(stderr, "Close down completed, all components disconnected, disabled and destroyed\n\n");
        }
    }

    RaspiVid* RaspiVid::create() {
        return RaspiVid::create(createRaspiVidDefaultState());
    }

    RaspiVid* RaspiVid::create(RASPIVID_STATE_S state) {
        if (!RaspiVid::singleton_) {
            RaspiVid::singleton_ = new RaspiVid();
        }
        RaspiVid::singleton_->state = state; 
        return RaspiVid::singleton_;
    }

    RaspiVid* RaspiVid::getInstance() {
        return RaspiVid::singleton_;
    }
    
    RaspiVid::RaspiVid() {
    }

    RaspiVid::~RaspiVid() {
        stop();
    }

    
}
