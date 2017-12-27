#include "rpi_motioncam/RPiMotionCam.h"

using namespace raspi_cam_control;

namespace rpi_motioncam {
    RPIMOTIONCAM_OPTION_S RPiMotionCam::createMotionCamDefaultOptions() {
        RPIMOTIONCAM_OPTION_S options;
        
        options.width = 1640;       
        options.height = 1232;

        /*
         options.width = 3240;
         options.height = 2464;
        */

        options.framerate = VIDEO_FRAME_RATE_NUM;

        options.verbose = true;
        options.cameraNum = 0;

        options.raw_output_fmt = RAW_OUTPUT_FMT_GRAY;
        options.preview = true;

        options.resizer_width = 640;
        options.resizer_height = 480;

        options.motion_threshold = MOTION_THRESHOLD_DEFAULT;

        return options;
    }

    bool RPiMotionCam::frame_ready() {
        return MotionData::has_ready_frame();
    }

    shared_ptr< MotionData > RPiMotionCam::get_frame() {
        shared_ptr< MotionData > frame;
        MotionData::get_ready_frame(frame);
        return frame;
    }

    MMAL_STATUS_T RPiMotionCam::create_components() {
        MMAL_STATUS_T status = MMAL_SUCCESS;
 
        RASPICAMERA_OPTION_S options = RaspiCamera::createDefaultCameraOptions();
        options.width = options_.width;
        options.height = options_.height;
        options.framerate = options_.framerate;
        options.verbose = options_.verbose;
        options.cameraNum = options_.cameraNum;

        camera = RaspiCamera::create(options);
        if (camera == NULL) {
            vcos_log_error("RPiMotionCam::create_components(): Failed to create camera");
            return MMAL_ENOSYS;
        }

        if (options_.preview) {
            preview_renderer = RaspiRenderer::create();
            if (preview_renderer == NULL) {
                vcos_log_error("RPiMotionCam::create_components(): Failed to create preview_renderer");
                return MMAL_ENOSYS;
            }
        }

        resizer = RaspiResize::create();
        if (resizer == NULL) {
            vcos_log_error("RPiMotionCam::create_components(): Failed to create resizer");
            return MMAL_ENOSYS;
        }

        // Set encoder width and height to match resizer width and height
        RASPIENCODER_OPTION_S encoder_options = RaspiEncoder::createDefaultEncoderOptions();
        encoder_options.width = options_.resizer_width;
        encoder_options.height = options_.resizer_height;
        encoder = RaspiEncoder::create(encoder_options);

        if (encoder == NULL) {
            vcos_log_error("RPiMotionCam::create_components(): Failed to create encoder");
            return MMAL_ENOSYS;
        }

        splitter = RaspiSplitter::create();
        if (splitter == NULL) {
            vcos_log_error("RPiMotionCam::create_components(): Failed to create splitter");
            return MMAL_ENOSYS;
        }

        nullsink = RaspiNullsink::create();
        if (nullsink == NULL) {
            vcos_log_error("RPiMotionCam::create_components(): Failed to create null sink");
        }

       
        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RPiMotionCam::connect_components() {
        MMAL_STATUS_T status;
        if (options_.preview) {
            if ((status = preview_renderer->input->connect(camera->preview)) != MMAL_SUCCESS) {
                vcos_log_error("RPiMotionCam::connect_components(): couldn't connect camera preview to preview renderer");
                return status;
            }
        } else {
            if ((status = nullsink->input->connect(camera->preview)) != MMAL_SUCCESS) {
                vcos_log_error("RPiMotionCam::connect_components(): couldn't connect nullsink input to camera preview");
                return status;
            }
        }
        if ((status = splitter->input->connect(camera->video)) != MMAL_SUCCESS) {
            vcos_log_error("RPiMotionCam::connect_components(): couldn't connect camera video to splitter");
            return status;
        }
        if ((status = splitter->duplicate_input()) != MMAL_SUCCESS) {
            vcos_log_error("RPiMotionCam::connect_components(): couldn't duplicate splitter input");
            return status;
        }
        if ((status = resizer->input->connect(splitter->output_1)) != MMAL_SUCCESS) {
            vcos_log_error("RPiMotionCam::connect_components(): couldn't connect splitter output_1 to resizer");
            return status;
        }
        if ((status = resizer->set_output(options_.resizer_width, options_.resizer_height)) != MMAL_SUCCESS) {
            vcos_log_error("RPiMotionCam::connect_components(): couldn't set resizer image size");
            return status;
        }
        if ((status = encoder->input->connect(resizer->output)) != MMAL_SUCCESS) {
            vcos_log_error("RPiMotionCam::connect_components(): couldn't connect splitter output_1 to encoder");
            return status;
        }

        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RPiMotionCam::add_callbacks() {
        MMAL_STATUS_T status = MMAL_SUCCESS;

        roCallback = shared_ptr< RawOutputCallback >( new RawOutputCallback(options_.width, options_.height) );
        if (splitter->output_0->add_callback(roCallback) != MMAL_SUCCESS) {
            vcos_log_error("RPiMotionCam::add_callbacks(): Could not add raw output callback");
        }
        if (options_.preview) {
            mvCallback = shared_ptr< MotionVectorPreviewCallback >( new MotionVectorPreviewCallback(options_.resizer_width, options_.resizer_height) );
            if (encoder->output->add_callback(mvCallback) != MMAL_SUCCESS) {
                vcos_log_error("RPiMotionCam::add_callbacks(): couldn't add motion vector preview callback");
            }
        } else {
            mvCallback = shared_ptr< MotionVectorCallback >( new MotionVectorCallback(options_.resizer_width, options_.resizer_height, options_.motion_threshold) );
            if (encoder->output->add_callback(mvCallback) != MMAL_SUCCESS) {
                vcos_log_error("RPiMotionCam::add_callbacks(): couldn't add motion vector callback");
            }
        }
        return MMAL_SUCCESS;

    }

    MMAL_STATUS_T RPiMotionCam::init() {
        MMAL_STATUS_T status = MMAL_SUCCESS;
        bcm_host_init();

        vcos_log_register("RPiMotionCam", VCOS_LOG_CATEGORY);

        if ((status = create_components()) != MMAL_SUCCESS) {
            vcos_log_error("RPiMotionCam::init(): Failed to create one or more components");
            return status;
        }

        if (options_.verbose) {
            fprintf(stderr, "Starting component connection stage\n");
        }

        if ((status = connect_components()) != MMAL_SUCCESS) {
            vcos_log_error("RPiMotionCam::init(): Failed to connect one or more components");
            return status;
        }

        if (options_.verbose) {
            fprintf(stderr, "Starting callback addition stage\n");
        }
        if ((status = add_callbacks()) != MMAL_SUCCESS) {
            vcos_log_error("RPiMotionCam::init(): Failed to add one or more callbacks");
            return status;

        }
        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RPiMotionCam::start() {
       return camera->start();
    }

    void RPiMotionCam::stop() {
        if (options_.verbose) {
            fprintf(stderr, "Close down completed, all components disconnected, disabled and destroyed\n\n");
        }
    }

    shared_ptr< RPiMotionCam > RPiMotionCam::create() {
        return RPiMotionCam::create(RPiMotionCam::createMotionCamDefaultOptions());
    }

    shared_ptr< RPiMotionCam > RPiMotionCam::create(RPIMOTIONCAM_OPTION_S options) {
        auto result = shared_ptr< RPiMotionCam >( new RPiMotionCam() );
        result->options_ = options; 

        MMAL_STATUS_T status;

        if ((status = result->init()) != MMAL_SUCCESS) {
            return nullptr;
        }
        return result;
    }

    bool RPiMotionCam::frame_ready() {
        return MotionData::has_ready_frame();
    }

    shared_ptr< MotionData > RPiMotionCam::get_frame() {
        shared_ptr< MotionData > result;
        if (MotionData::get_ready_frame(result)) {
            return result;
        }
        return nullptr;
    }

    RPiMotionCam::RPiMotionCam() {
    }

    RPiMotionCam::~RPiMotionCam() {
        stop();
    }
}
