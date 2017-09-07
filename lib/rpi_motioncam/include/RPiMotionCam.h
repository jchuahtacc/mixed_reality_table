#ifndef __RPIMOTIONCAM_HPP__
#define __RPIMOTIONCAM_HPP__

#include "bcm_host.h"
#include "interface/vcos/vcos.h"
#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_types.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"

namespace rpi_motioncam {
    class RPiMotionCam {
        public:
            RPiMotionCam(int width = 640, int height = 480, bool preview = false);
            ~RPiMotionCam();
            MMAL_STATUS_T init();

        private:
            int width_ = 640;
            int height_ = 480;
            bool preview_ = false;

            MMAL_COMPONENT_T *camera_component;
            MMAL_COMPONENT_T *splitter_component;
            MMAL_COMPONENT_T *encoder_component;
            MMAL_COMPONENT_T *preview_component;
            MMAL_COMPONENT_T *null_sink_component;

            MMAL_PORT_T *camera_preview_port = NULL;
            MMAL_PORT_T *camera_video_port = NULL;
            MMAL_PORT_T *camera_still_port = NULL;
            MMAL_PORT_T *preview_input_port = NULL;
            MMAL_PORT_T *encoder_input_port = NULL;
            MMAL_PORT_T *encoder_output_port = NULL;
            MMAL_PORT_T *splitter_input_port = NULL;
            MMAL_PORT_T *splitter_output_port = NULL;
            MMAL_PORT_T *splitter_preview_port = NULL;

            MMAL_STATUS_T create_camera_component();
            MMAL_STATUS_T create_preview_component();
            MMAL_STATUS_T create_splitter_component();
            MMAL_STATUS_T create_encoder_component();
            MMAL_STATUS_T create_null_sink_component();

            void destroy_camera_component();
            void destroy_preview_component();
            void destroy_splitter_component();
            void destroy_encoder_component();
            void destroy_null_sink_component();

    };
}

#endif
