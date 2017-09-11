#include "../include/RPiMotionCam.h"
#include <iostream>
#include <functional>

using namespace std;
using namespace std::placeholders;

namespace rpi_motioncam {

    RPiMotionCam::RPiMotionCam() {
    }

    RPiMotionCam::~RPiMotionCam() {
        destroy_camera_ports();
        destroy_connections();
        disable_components();
        destroy_components();
    }
    
    RPiMotionCam *RPiMotionCam::instance_ = NULL;

    RPiMotionCam* RPiMotionCam::create() {
        if (RPiMotionCam::instance_ == NULL) {
            RPiMotionCam::instance_ = new RPiMotionCam();
        }
        return RPiMotionCam::instance_;
    }

    MMAL_STATUS_T RPiMotionCam::init() {
        return init(PICAM_DEFAULT);
    }

    MMAL_STATUS_T RPiMotionCam::init(int mode) {
        return init(mode, false);
    }

    MMAL_STATUS_T RPiMotionCam::init(int mode, bool preview) {
        if (!setCameraMode(mode)) {
            // Return error on bad camera mode 
            return MMAL_ECONFIG;
        }
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
        MMAL_PARAMETER_CHANGE_EVENT_REQUEST_T change_event_request =
            {{MMAL_PARAMETER_CHANGE_EVENT_REQUEST, sizeof(MMAL_PARAMETER_CHANGE_EVENT_REQUEST_T)},
            MMAL_PARAMETER_CAMERA_SETTINGS, 1};

        if ((status = mmal_port_parameter_set(camera_component->control, &change_event_request.hdr)) != MMAL_SUCCESS) {
            vcos_log_error("No camera settings events");
        }

        
        // Enable camera, add callback function
        if ((status = mmal_port_enable(camera_component->control, RPiMotionCam::camera_control_callback)) != MMAL_SUCCESS) {
            vcos_log_error("create_camera_component() could not enable control port : error %d", status);
            return status;
        }


        // Setup still camera format

        {
            MMAL_PARAMETER_CAMERA_CONFIG_T cam_config =
            {
                { MMAL_PARAMETER_CAMERA_CONFIG, sizeof(cam_config) },
                  .max_stills_w = width_,
                  .max_stills_h = height_,
                  .stills_yuv422 = 0,
                  .one_shot_stills = 0,
                  .max_preview_video_w = width_,
                  .max_preview_video_h = height_,
                  .num_preview_video_frames = 3 + vcos_max(0, (fps_ - 30) / 10),
                  .stills_capture_circular_buffer_height = 0,
                  .fast_preview_resume = 0,
                  .use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RAW_STC
            };
            mmal_port_parameter_set(camera_component->control, &cam_config.hdr);
        }

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

        format->encoding = MMAL_ENCODING_OPAQUE;
        format->es->video.width = VCOS_ALIGN_UP(width_, 32);
        format->es->video.height = VCOS_ALIGN_UP(height_, 16);
        format->es->video.crop.x = 0;
        format->es->video.crop.y = 0;
        format->es->video.crop.width = width_;
        format->es->video.crop.height = height_;
        // Variable preview frame rate
        format->es->video.frame_rate.num = 0;
        format->es->video.frame_rate.den = 1;

        if ((status = mmal_port_format_commit(camera_preview_port)) != MMAL_SUCCESS) {
            vcos_log_error("create_camera_component(): couldn't set video format");
            return status;
        }
/*
        // Don't care about the still port
        if (camera_still_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM) {
            camera_still_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;
        }
*/
        
        if ((status = mmal_component_enable(camera_component)) != MMAL_SUCCESS) {
            vcos_log_error("create_camera_component(): camera component couldn't be enabled");
            return status;
        }

        // Set a billion camera parameters. Ignoring for now...
        // raspicamcontrol_set_all_parameters(camera, &state->camera_parameters);

        return MMAL_SUCCESS;
    }

    bool RPiMotionCam::setCameraMode(int mode) {
        camera_mode_ = mode;
        switch (camera_mode_) {
            case PICAM_DEFAULT:
                width_ = 640;
                height_ = 480;
                fps_ = 60;
                camera_mode_ = 0;
                return true;
                break;
            case PICAM1X_1920_1080_30:
                width_ = 1920;
                height_ = 1080;
                fps_ = 30;
                camera_mode_ = 1;
                return true;
                break;
            case PICAM1X_2592_1944_30:
                width_ = 2592;
                height_= 1944;
                fps_ = 30;
                camera_mode_ = 2;
                return true;
                break;
            case PICAM1X_1296_972_42:
                width_ = 1296;
                height_ = 972;
                fps_ = 42;
                camera_mode_ = 4;
                return true;
                break;
            case PICAM1X_1296_730_49:
                width_ = 1296;
                height_ = 730;
                fps_ = 49;
                camera_mode_ = 5;
                return true;
                break;
            case PICAM1X_640_480_60:
                width_ = 640;
                height_ = 480;
                fps_ = 60;
                camera_mode_ = 6;
                return true;
                break;
            case PICAM1X_640_480_90:
                width_ = 640;
                height_ = 480;
                fps_ = 90;
                camera_mode_ = 7;
                return true;
                break;
            case PICAM2X_1920_1080_30:
                width_ = 1920;
                height_ = 1080;
                fps_ = 30;
                camera_mode_ = 1;
                return true;
                break;
            case PICAM2X_3280_2464_15:
                width_ = 3280;
                height_ = 2464;
                fps_ = 15;
                // Or mode 3? ...?
                camera_mode_ = 2;
                return true;
                break;
            case PICAM2X_1640_1232_40:
                width_ = 1640;
                height_ = 1232;
                fps_ = 40;
                camera_mode_ = 4;
                return true;
                break;
            case PICAM2X_1640_922_40:
                width_ = 1640;
                height_ = 922;
                fps_ = 40;
                camera_mode_ = 5;
                return true;
                break;
            case PICAM2X_1280_720_90:
                width_ = 1280;
                height_ = 720;
                fps_ = 90;
                camera_mode_ = 6;
                return true;
                break;
            case PICAM2X_640_480_90:
                width_ = 640;
                height_ = 480;
                fps_ = 90;
                camera_mode_ = 7;
                return true;
                break;
            default:
                // Invalid mode
                return false;
        }
        return false;
    }

    int RPiMotionCam::getWidth() {
        return width_;
    }

    int RPiMotionCam::getHeight() {
        return height_;
    }

    int RPiMotionCam::getFPS() {
            return fps_;
    }

    MMAL_STATUS_T RPiMotionCam::create_preview_component() {
        MMAL_STATUS_T status;

        // Create preview renderer
        if ((status = mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_RENDERER, &preview_component)) != MMAL_SUCCESS) {
            vcos_log_error("create_preview_component(): could not create preview component"); 
            return status;
        }

        // Sanity check on preview renderer
        if (!preview_component->input_num) {
            vcos_log_error("create_preview_component(): no input ports found on preview component (?!?!)");
            return MMAL_ENOSYS;
        }

        preview_input_port = preview_component->input[0];

        MMAL_DISPLAYREGION_T param;
        param.hdr.id = MMAL_PARAMETER_DISPLAYREGION;
        param.hdr.size = sizeof(MMAL_DISPLAYREGION_T);
        param.set = MMAL_DISPLAY_SET_LAYER;
        param.layer = PREVIEW_LAYER;
        param.set |= MMAL_DISPLAY_SET_ALPHA;
        // Opaque preview
        param.alpha = 255;
        param.set |= MMAL_DISPLAY_SET_FULLSCREEN;
        param.fullscreen = 1;


        status = mmal_port_parameter_set(preview_input_port, &param.hdr);
        if (status != MMAL_SUCCESS && status != MMAL_ENOSYS) {
            vcos_log_error("create_preview_component(): unable to set preview port parameters (%u)", status);
            return status;
        }

        if ((status = mmal_component_enable(preview_component)) != MMAL_SUCCESS) {
            vcos_log_error("create_preview_component(): unable to enable preview component (%u)", status);
            return status;
        }

        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RPiMotionCam::create_splitter_component() {
        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RPiMotionCam::create_encoder_component() {
        MMAL_STATUS_T status;

        if ((status = mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER, &encoder_component)) != MMAL_SUCCESS) {
            vcos_log_error("create_encoder_component(): unable to create video encoder component");
            return status;
        }

        // Sanity check that encoder component has inputs and outputs
        if (!encoder_component->input_num || !encoder_component->output_num) {
            vcos_log_error("create_encoder_component(): encoder doesn't have input/output ports");
            vcos_log_error("create_encoder_component(): %u inputs, %u outputs", encoder_component->input_num, encoder_component->output_num);
            return MMAL_ENOSYS;
        }

        encoder_input_port = encoder_component->input[0];
        encoder_output_port = encoder_component->output[0];

        mmal_format_copy(encoder_output_port->format, encoder_input_port->format);

        // Require H264 encoding for motion vectors
        encoder_output_port->format->encoding = MMAL_ENCODING_H264;
        encoder_output_port->format->bitrate = MAX_BITRATE_LEVEL_4;
        encoder_output_port->buffer_size = encoder_output_port->buffer_size_recommended;
        if (encoder_output_port->buffer_size < encoder_output_port->buffer_size_min) {
            encoder_output_port->buffer_size = encoder_output_port->buffer_size_min;
        }
        encoder_output_port->buffer_num = encoder_output_port->buffer_num_recommended;
        if (encoder_output_port->buffer_num < encoder_output_port->buffer_num_min) {
            encoder_output_port->buffer_num = encoder_output_port->buffer_num_min;
        }
        encoder_output_port->format->es->video.frame_rate.num = 0;
        encoder_output_port->format->es->video.frame_rate.den = 1;

        if ((status = mmal_port_format_commit(encoder_output_port)) != MMAL_SUCCESS) {
            vcos_log_error("create_encoder_component(): unable to set format on video encoder output port");
            return status;
        }

        // Omitting intraperiod, quantization code ...

        MMAL_PARAMETER_VIDEO_PROFILE_T param;
        param.hdr.id = MMAL_PARAMETER_PROFILE;
        param.hdr.size = sizeof(param);
        param.profile[0].profile = MMAL_VIDEO_PROFILE_H264_BASELINE;
        h264level_ = MMAL_VIDEO_LEVEL_H264_4;
        if ((VCOS_ALIGN_UP(width_, 16) >> 4) * (VCOS_ALIGN_UP(height_, 16) >> 4) * fps_ > 245760) {
            if ((VCOS_ALIGN_UP(width_, 16) >> 4) * (VCOS_ALIGN_UP(height_, 16) >> 4) * fps_ <= 522240) {
                // increase H264 level to 4.2... may not do anything for us
                h264level_ = MMAL_VIDEO_LEVEL_H264_42;
            } else {
                vcos_log_error("create_encoder_component(): too many macroblocks per second requested");
                return MMAL_ENOSYS;
            }
        }
        param.profile[0].level = h264level_;
        if ((status = mmal_port_parameter_set(encoder_output_port, &param.hdr)) != MMAL_SUCCESS) {
            vcos_log_error("create_encoder_component(): unable to set h264 profile");
            return status;
        }

        if ((status = mmal_port_parameter_set_boolean(encoder_output_port, MMAL_PARAMETER_VIDEO_ENCODE_INLINE_VECTORS, true)) != MMAL_SUCCESS) {
            vcos_log_error("create_encoder_component(): could not set inline motion vecotrs");
            return status;
        }

        if ((status = mmal_component_enable(encoder_component)) != MMAL_SUCCESS) {
            vcos_log_error("create_encoder_component(): could not enable video encoder component");
            return status;
        }
        
        encoder_pool = mmal_port_pool_create(encoder_output_port, encoder_output_port->buffer_num, encoder_output_port->buffer_size);
        if (!encoder_pool) {
            vcos_log_error("create_encoder_component(): could not create encoder buffer pool");
            return MMAL_ENOSYS;
        }

        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RPiMotionCam::create_null_sink_component() {
        MMAL_STATUS_T status;
        if ((status = mmal_component_create("vc.null_sink", &null_sink_component)) != MMAL_SUCCESS) {
            vcos_log_error("create_null_sink_component(): could not create null sink component");
            return status;
        }
        if (!null_sink_component->input_num) {
            vcos_log_error("create_null_sink_component(): null sink component had no inputs");
            return MMAL_ENOSYS;
        }
        null_sink_input_port = null_sink_component->input[0];

        return MMAL_SUCCESS;
    }

    void RPiMotionCam::check_disable_port(MMAL_PORT_T *port) {
        if (port && port->is_enabled) {
            mmal_port_disable(port);
        }
    }

    void RPiMotionCam::destroy_camera_ports() {
        check_disable_port(camera_still_port);
        check_disable_port(encoder_output_port);
        check_disable_port(splitter_output_port);
    }

    void RPiMotionCam::destroy_connections() {
        if (preview_connection) {
            mmal_connection_destroy(preview_connection);
        }
        if (encoder_connection) {
            mmal_connection_destroy(encoder_connection);
        }
        if (splitter_connection) {
            mmal_connection_destroy(splitter_connection);
        }
    }

    void RPiMotionCam::disable_components() {
        if (null_sink_component) {
            mmal_component_disable(null_sink_component);
        }
        if (encoder_component) {
            mmal_component_disable(encoder_component);
        }
        if (preview_component) {
            mmal_component_disable(preview_component);
        }
        if (splitter_component) {
            mmal_component_disable(splitter_component);
        }
        if (camera_component) {
            mmal_component_disable(camera_component);
        }
    }

    void RPiMotionCam::destroy_components() {
        if (encoder_pool) {
            mmal_port_pool_destroy(encoder_component->output[0], encoder_pool);
        }
        if (encoder_component) {
            mmal_component_destroy(encoder_component);
            encoder_component = NULL;
        }
        if (preview_component) {
            mmal_component_destroy(preview_component);
            preview_component = NULL;
        }
        if (null_sink_component) {
            mmal_component_destroy(null_sink_component);
            null_sink_component = NULL;
        }
        if (splitter_component) {
            mmal_component_destroy(splitter_component);
            splitter_component = NULL;
        }

        if (camera_component) {
            mmal_component_destroy(camera_component);
            camera_component = NULL;
        }
        
    }


    void RPiMotionCam::camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
        cerr << "camera_control_callback(): I mostly don't care" << endl;
        mmal_buffer_header_release(buffer);
    }
}
