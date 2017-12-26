#ifndef _GNU_SOURCE
   #define _GNU_SOURCE
#endif

#include "rpi_motioncam/RPiMotionCam.h"

using namespace raspi_cam_control;
using namespace raspivid;

namespace rpi_motioncam {

    shared_ptr< RPiMotionCam > RPiMotionCam::singleton_ = nullptr;

    RPI_MOTIONCAM_OPTION_S RPiMotionCam::createMotionCamDefaultOptions() {
        RPI_MOTIONCAM_OPTION_S  options;
        
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
        if (camera == nullptr ) {
            vcos_log_error("RPiMotionCam::create_components(): Failed to create camera");
            return MMAL_ENOSYS;
        }

        RASPIPORT_FORMAT_S format = camera->video->get_format();
        format.encoding = MMAL_ENCODING_I420;
        if ((status = camera->video->set_format(format)) != MMAL_SUCCESS) {
            vcos_log_error("RPimotionCam::create_components(): Failed to set camera video port format");
            return status;
        }

        if (options_.preview) {
            preview_renderer = RaspiRenderer::create();
            if (preview_renderer == nullptr ) {
                vcos_log_error("RPiMotionCam::create_components(): Failed to create preview_renderer");
                return MMAL_ENOSYS;
            }
        }

        resizer = RaspiResize::create(options_.resizer_width, options_.resizer_height);
        if (resizer == nullptr) {
            vcos_log_error("RPiMotionCam::create_components(): Failed to create resizer");
            return MMAL_ENOSYS;
        }

        // Set encoder width and height to match resizer width and height
        RASPIENCODER_OPTION_S encoder_options = RaspiEncoder::createDefaultEncoderOptions();
        encoder_options.width = options_.resizer_width;
        encoder_options.height = options_.resizer_height;
        encoder = RaspiEncoder::create(encoder_options);

        if (encoder == nullptr) {
            vcos_log_error("RPiMotionCam::create_components(): Failed to create encoder");
            return MMAL_ENOSYS;
        }

        splitter = RaspiSplitter::create();
        if (splitter == nullptr) {
            vcos_log_error("RPiMotionCam::create_components(): Failed to create splitter");
            return MMAL_ENOSYS;
        }

        preview_nullsink = RaspiNullsink::create();
        if (preview_nullsink == nullptr) {
            vcos_log_error("RPiMotionCam::create_components(): Failed to create null sink");
        }

        still_nullsink = RaspiNullsink::create();
        if (still_nullsink == nullptr) {
            vcos_log_error("RPiMotionCam::create_components(): Failed to create null sink");
        }
       
        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RPiMotionCam::connect_components() {
        MMAL_STATUS_T status;

        if ((status = still_nullsink->connect(camera->still)) != MMAL_SUCCESS) {
            vcos_log_error("RPiMotionCam::connect_components(): couldn't connect camera still port to nullsink");
            return status;
        }

        if (options_.preview) {
            if ((status = preview_renderer->connect(camera->preview)) != MMAL_SUCCESS) {
                vcos_log_error("RPiMotionCam::connect_components(): couldn't connect camera preview to preview renderer");
                return status;
            }
        } else {
            if ((status = preview_nullsink->connect(camera->preview)) != MMAL_SUCCESS) {
                vcos_log_error("RPiMotionCam::connect_components(): couldn't connect nullsink input to camera preview");
                return status;
            }
        }
        if ((status = splitter->connect(camera)) != MMAL_SUCCESS) {
            vcos_log_error("RPiMotionCam::connect_components(): couldn't connect camera video to splitter");
            return status;
        }
        if ((status = resizer->connect(splitter)) != MMAL_SUCCESS) {
            vcos_log_error("RPiMotionCam::connect_components(): couldn't connect splitter output_0 to resizer");
            return status;
        }
        if ((status = encoder->connect(resizer)) != MMAL_SUCCESS) {
            vcos_log_error("RPiMotionCam::connect_components(): couldn't connect splitter output_0 to encoder");
            return status;
        }
        return MMAL_SUCCESS;
    }

    MMAL_STATUS_T RPiMotionCam::add_callbacks() {
        MMAL_STATUS_T status = MMAL_SUCCESS;


        roCallback = shared_ptr< RawOutputCallback >( new RawOutputCallback(options_.width, options_.height) );
        if (splitter->output_1->add_callback(roCallback) != MMAL_SUCCESS) {
            vcos_log_error("RPiMotionCam::add_callbacks(): Could not add raw output callback");
        }
        if (options_.preview) {
            mvCallback = shared_ptr< MotionVectorPreviewCallback >( new MotionVectorPreviewCallback(options_.resizer_width, options_.resizer_height) );
            if (encoder->output->add_callback(mvCallback) != MMAL_SUCCESS) {
                vcos_log_error("RPiMotionCam::add_callbacks(): couldn't add motion vector preview callback");
            }
        } else {
            mvCallback = shared_ptr< MotionVectorCallback >( new MotionVectorCallback(options_.resizer_width, options_.resizer_height) );
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
            vcos_log_error("RPiMotionCam::init():Starting component connection");
        }

        if ((status = connect_components()) != MMAL_SUCCESS) {
            vcos_log_error("RPiMotionCam::init(): Failed to connect one or more components");
            return status;
        }

        if (options_.verbose) {
            vcos_log_error("RPiMotionCam::init(): Starting callback addition");
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

    shared_ptr< RPiMotionCam > RPiMotionCam::create(RPI_MOTIONCAM_OPTION_S options) {
        if (!RPiMotionCam::singleton_) {
            RPiMotionCam::singleton_ = shared_ptr< RPiMotionCam >( new RPiMotionCam() );
        }
        RPiMotionCam::singleton_->options_ = options; 
        MMAL_STATUS_T status;
        if ((status = RPiMotionCam::singleton_->init()) != MMAL_SUCCESS) {
            return nullptr;
        }
        return RPiMotionCam::singleton_;
    }

    shared_ptr< RPiMotionCam > RPiMotionCam::getInstance() {
        return RPiMotionCam::singleton_;
    }
    
    RPiMotionCam::RPiMotionCam() {
    }

    RPiMotionCam::~RPiMotionCam() {
        stop();
    }
}
