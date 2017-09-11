#include "../include/RPiMotionCam.h"
#include <iostream>
#include <functional>

using namespace std;
using namespace std::placeholders;

namespace rpi_motioncam {

    RPiMotionCam::RPiMotionCam() {
    }

    RPiMotionCam::~RPiMotionCam() {
        destroy_camera_component();
        destroy_preview_component();
        destroy_splitter_component();
        destroy_encoder_component();
        destroy_null_sink_component();
    }
    
    RPiMotionCam *RPiMotionCam::instance_ = NULL;

    RPiMotionCam* RPiMotionCam::create() {
        if (RPiMotionCam::instance_ == NULL) {
            RPiMotionCam::instance_ = new RPiMotionCam();
        }
        return RPiMotionCam::instance_;
    } 

    MMAL_STATUS_T RPiMotionCam::init(int mode, bool preview = false) {
        camera_mode_ = mode;
        preview_ = preview;
        bcm_host_init();
        vcos_log_register("rpi_motioncam", VCOS_LOG_CATEGORY);
        MMAL_STATUS_T status;

        // Create each component, abort if any fail
        if ((status = create_camera_component()) != MMAL_SUCCESS) {
            vcos_log_error("Failed to create camera component");
            return status;
        }

        if (preview_) {
            if ((status = create_preview_component()) != MMAL_SUCCESS) {
                vcos_log_error("Failed to create preview component");
                return status;
            }
        }

        if ((status = create_splitter_component()) != MMAL_SUCCESS) {
            vcos_log_error("Failed to create splitter component");
            return status;
        }

        if ((status = create_encoder_component()) != MMAL_SUCCESS) {
            vcos_log_error("Failed to create encoder component");
            return status;
        }

        if ((status = create_null_sink_component()) != MMAL_SUCCESS) {
            vcos_log_error("Failed to create null sink component");
            return status;
        }

        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RPiMotionCam::create_camera_component() {
        MMAL_STATUS_T status;

        // Create default camera component
        if ((status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera_component)) != MMAL_SUCCESS) {
            vcos_log_error("create_camera_component() failed to create default camera");
            return status;
        }

        // Select camera number 0
        MMAL_PARAMETER_INT32_T camera_num = {{MMAL_PARAMETER_CAMERA_NUM, sizeof(camera_num)}, 0};
        if ((status = mmal_port_parameter_set(camera_component->control, &camera_num.hdr)) != MMAL_SUCCESS) {
            vcos_log_error("create_camera_component() fould not select camera : error %d", status);
            return status;
        }

        // Sanity check to make sure camera has outputs
        if (!camera_component->output_num) {
            vcos_log_error("create_camera_component() camera has no outputs - very strange!");
            return MMAL_ENOSYS;
        }

        // Set camera mode (resolution/fps)
        if ((status = mmal_port_parameter_set_uint32(camera_component->control, MMAL_PARAMETER_CAMERA_CUSTOM_SENSOR_CONFIG, camera_mode_)) != MMAL_SUCCESS) {
            vcos_log_error("create_camera_component() could not set sensor mode : error %d", status);
            return status;
        }

        // Save camera output ports
        camera_preview_port = camera_component->output[0];
        camera_video_port = camera_component->output[1];
        camera_still_port = camera_component->output[2];

        // Request camera settings events
        /*
        
            MMAL_PARAMETER_CHANGE_EVENT_REQUEST_T change_event_request =
                {{MMAL_PARAMETER_CHANGE_EVENT_REQUEST, sizeof(MMAL_PARAMETER_CHANGE_EVENT_REQUEST_T)},
                MMAL_PARAMETER_CAMERA_SETTINGS, 1};

            status = mmal_port_parameter_set(camera->control, &change_event_request.hdr);
            if ( status != MMAL_SUCCESS ) {
                vcos_log_error("No camera settings events");
            }

        */
        
        // Enable camera, add callback function
        if ((status = mmal_port_enable(camera_component->control, RPiMotionCam::camera_control_callback)) != MMAL_SUCCESS) {
            vcos_log_error("create_camera_component() could not enable control port : error %d", status);
            return status;
        }

        /*

        // Disabled - setup still camera format

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
        */

        // Setup port formats
        
        format = camera_preview_port->format;
        format->encoding = MMAL_ENCODING_OPAQUE;
        format->encoding_variant = MMAL_ENCODING_I420;
        
        /*
           // Shutter speed code

            if (state->camera_parameters.shutter_speed > 6000000) {
                MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)}, { 50, 1000 }, {166, 1000}};
                mmal_port_parameter_set(preview_port, &fps_range.hdr);
            } else if (state->camera_parameters.shutter_speed > 1000000) {
                MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)}, { 166, 1000 }, {999, 1000}};
                mmal_port_parameter_set(preview_port, &fps_range.hdr);
            }

            // enable dynamic framerate if necessary
            if (state->camera_parameters.shutter_speed) {
                if (state->framerate > 1000000./state->camera_parameters.shutter_speed) {
                    state->framerate=0;
                    if (state->verbose) {
                        fprintf(stderr, "Enable dynamic frame rate to fulfil shutter speed requirement\n");
                    }
                }
            }
        */

        return MMAL_SUCCESS;
    }

    int RPiMotionCam::getWidth() {
        return width_;
    }

    int RPiMotionCam::getHeight() {
        return height_;
    }

    MMAL_STATUS_T RPiMotionCam::create_preview_component() {
        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RPiMotionCam::create_splitter_component() {
        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RPiMotionCam::create_encoder_component() {
        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RPiMotionCam::create_null_sink_component() {
        return MMAL_SUCCESS;
    }

    void RPiMotionCam::destroy_camera_component() {
        if (camera_component) {
        }
    }
    
    void RPiMotionCam::destroy_preview_component() {
        if (preview_component) {
        }
    }

    void RPiMotionCam::destroy_splitter_component() {
        if (splitter_component) {
        }
    }

    void RPiMotionCam::destroy_encoder_component() {
        if (encoder_component) {
        }
    }

    void RPiMotionCam::destroy_null_sink_component() {
        if (null_sink_component) {
        }
    }


    void RPiMotionCam::camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
        cerr << "camera_control_callback(): I mostly don't care" << endl;
        mmal_buffer_header_release(buffer);
    }
}
