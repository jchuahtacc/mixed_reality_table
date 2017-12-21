#ifndef _GNU_SOURCE
   #define _GNU_SOURCE
#endif

#include "RaspiVid.h"

using namespace raspi_cam_control;

namespace raspivid {

    shared_ptr< RaspiVid > RaspiVid::singleton_ = nullptr;

    RASPIVID_OPTIONS_S RaspiVid::createRaspiVidDefaultOptions() {
        RASPIVID_OPTIONS_S options;
        options.width = 1640;       // Default to 1080p
        options.height = 1232;
        options.framerate = VIDEO_FRAME_RATE_NUM;

        options.verbose = true;
        options.cameraNum = 0;

        options.raw_output_fmt = RAW_OUTPUT_FMT_GRAY;
        options.raw_output = true;
        options.preview = true;

        options.resizer_width = 640;
        options.resizer_height = 480;

        return options;
    }

    MMAL_STATUS_T RaspiVid::create_components() {
        MMAL_STATUS_T status = MMAL_SUCCESS;
 
        RASPICAMERA_OPTION_S options = RaspiCamera::createDefaultCameraOptions();
        options.width = options_.width;
        options.height = options_.height;
        options.framerate = options_.framerate;
        options.verbose = options_.verbose;
        options.cameraNum = options_.cameraNum;

        camera = RaspiCamera::create(options);
        if (camera == NULL) {
            vcos_log_error("RaspiVid::create_components(): Failed to create camera");
            return MMAL_ENOSYS;
        }

        if (options_.preview) {
            preview_renderer = RaspiRenderer::create();
            if (preview_renderer == NULL) {
                vcos_log_error("RaspiVid::create_components(): Failed to create preview_renderer");
                return MMAL_ENOSYS;
            }
        }

        resizer = RaspiResize::create();
        if (resizer == NULL) {
            vcos_log_error("RaspiVid::create_components(): Failed to create resizer");
            return MMAL_ENOSYS;
        }

        // Set encoder width and height to match resizer width and height
        RASPIENCODER_OPTION_S encoder_options = RaspiEncoder::createDefaultEncoderOptions();
        encoder_options.width = options_.resizer_width;
        encoder_options.height = options_.resizer_height;
        encoder = RaspiEncoder::create(encoder_options);

        if (encoder == NULL) {
            vcos_log_error("RaspiVid::create_components(): Failed to create encoder");
            return MMAL_ENOSYS;
        }

        splitter = RaspiSplitter::create();
        if (splitter == NULL) {
            vcos_log_error("RaspiVid::create_components(): Failed to create splitter");
            return MMAL_ENOSYS;
        }

       
        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RaspiVid::connect_components() {
        MMAL_STATUS_T status;
        if (options_.preview) {
            if (options_.raw_output) {
                vcos_assert(splitter);
                status = splitter->input->connect(camera->preview);
                if (status != MMAL_SUCCESS) {
                    vcos_log_error("RaspiVid::connect_components(): failed to connect camera preview to splitter input");
                    return status;
                }

                if (splitter->duplicate_input() != MMAL_SUCCESS) {
                    vcos_log_error("RaspiVid::connect_components(): unable to copy splitter input format to outputs");
                }
                
                // Connect splitter to preview
                status = preview_renderer->input->connect(splitter->output_1);
                if (status != MMAL_SUCCESS) {
                    vcos_log_error("RaspiVid::connect_components(): failed to connect preview_renderer");
                    return status;
                }
            } else {
                // Want preview, don't want raw output
                if (options_.verbose) {
                   fprintf(stderr, "Connecting camera preview port to preview input port\n");
                   fprintf(stderr, "Starting video preview\n");
                }

                // Connect camera to preview
                status = preview_renderer->input->connect(camera->preview);
                if (status != MMAL_SUCCESS) {
                    vcos_log_error("RaspiVid::connect_components: Failed to connect camera preview to preview input port");
                    return status;
                }
            }
        } else {
            if (options_.raw_output) {
                // Don't want preview, but want raw output
                if (options_.verbose) {
                    fprintf(stderr, "Connecting camera preview port to splitter input port\n");
                }

                // Connect camera to splitter
                status = splitter->input->connect(camera->preview);
                if (status != MMAL_SUCCESS) {
                    vcos_log_error("RaspiVid::connect_components(): Failed to connect camera preview port to splitter input");
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

        if (options_.verbose)
            fprintf(stderr, "Connecting camera video port to resizer input port\n");

        // Connect camera to resizer
        vcos_assert(resizer->input);
        status = resizer->input->connect(camera->video);
        if (status != MMAL_SUCCESS) {
            vcos_log_error("RaspiVid::connect_components(): Failed to connect RaspiResizer to camera_video_port");
            return status;
        }
        status = resizer->set_output(options_.resizer_width, options_.resizer_height);
        if (status != MMAL_SUCCESS) {
            vcos_log_error("RaspiVid::connect_components(): Failed to set output format on resizer");
            return status;
        }
        
        // Connect resizer to encoder
        vcos_assert(encoder->input);
        vcos_assert(resizer->output);
        status = encoder->input->connect(resizer->output);
        if (status != MMAL_SUCCESS) {
            vcos_log_error("RaspiVid::connect_components(): Failed to connect RaspiEncoder to RaspiResizer");
            return status;
        }

        return MMAL_SUCCESS;
    }


    MMAL_STATUS_T RaspiVid::add_callbacks() {
        MMAL_STATUS_T status = MMAL_SUCCESS;

        if (options_.raw_output) {
            roCallback = shared_ptr< RawOutputCallback >( new RawOutputCallback() );
            if (splitter->output_0->add_callback(roCallback) != MMAL_SUCCESS) {
                vcos_log_error("RaspiVid::add_callbacks(): Could not add raw output callback");
            }
        }

        mvCallback = shared_ptr< MotionVectorPreviewCallback >( new MotionVectorPreviewCallback(options_.resizer_width, options_.resizer_height) );
        encoder->output->add_callback(mvCallback);
        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RaspiVid::init() {
        MMAL_STATUS_T status = MMAL_SUCCESS;
        bcm_host_init();

        vcos_log_register("RaspiVid", VCOS_LOG_CATEGORY);

        if ((status = create_components()) != MMAL_SUCCESS) {
            vcos_log_error("RaspiVid::init(): Failed to create one or more components");
            return status;
        }

        if (options_.verbose) {
            fprintf(stderr, "Starting component connection stage\n");
        }

        if ((status = connect_components()) != MMAL_SUCCESS) {
            vcos_log_error("RaspiVid::init(): Failed to connect one or more components");
            return status;
        }

        if (options_.verbose) {
            fprintf(stderr, "Starting callback addition stage\n");
        }

        if ((status = add_callbacks()) != MMAL_SUCCESS) {
            vcos_log_error("RaspiVid::init(): Failed to add one or more callbacks");
            return status;

        }
        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RaspiVid::start() {
       return camera->start();
    }

    void RaspiVid::stop() {
        if (options_.verbose) {
            fprintf(stderr, "Close down completed, all components disconnected, disabled and destroyed\n\n");
        }
    }

    shared_ptr< RaspiVid > RaspiVid::create() {
        return RaspiVid::create(RaspiVid::createRaspiVidDefaultOptions());
    }

    shared_ptr< RaspiVid > RaspiVid::create(RASPIVID_OPTIONS_S options) {
        if (!RaspiVid::singleton_) {
            RaspiVid::singleton_ = shared_ptr< RaspiVid >( new RaspiVid() );
        }
        RaspiVid::singleton_->options_ = options; 
        return RaspiVid::singleton_;
    }

    shared_ptr< RaspiVid > RaspiVid::getInstance() {
        return RaspiVid::singleton_;
    }
    
    RaspiVid::RaspiVid() {
    }

    RaspiVid::~RaspiVid() {
        stop();
    }
}
