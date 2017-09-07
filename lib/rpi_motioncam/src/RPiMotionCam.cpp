#include "../include/RPiMotionCam.h"
#include <iostream>

using namespace std;

namespace rpi_motioncam {
    RPiMotionCam::RPiMotionCam(int width, int height, bool preview) : width_(width), height_(height), preview_(preview) {
        cout << "Initializing RPiMotionCam at " << width_ << "x" << height_ << endl;
        
    }

    RPiMotionCam::~RPiMotionCam() {
        destroy_camera_component();
        destroy_preview_component();
        destroy_splitter_component();
        destroy_encoder_component();
        destroy_null_sink_component();
    }

    MMAL_STATUS_T RPiMotionCam::init() {
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
        return MMAL_SUCCESS;
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
}
