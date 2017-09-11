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

// Camera capture modes (resolutions and frame rates)
// https://www.raspberrypi.org/documentation/raspbian/applications/camera.md
#define     PICAM1X_AUTO            0
#define     PICAM1X_1920_1080_30    1
#define     PICAM1X_2592_1944_30    2
#define     PICAM1X_2592_1944_1     3
#define     PICAM1X_1296_972_42     4
#define     PICAM1X_1296_730_49     5
#define     PICAM1X_640_480_60      6
#define     PICAM1X_640_480_90      7
#define     PICAM2X_AUTO            0
#define     PICAM2X_1920_1080_30    1
#define     PICAM2X_3280_2464_15    2
#define     PICAM2X_1640_1232_40    4
#define     PICAM2X_1640_922_40     5
#define     PICAM2X_1280_720_90     6
#define     PICAM2X_640_480_90      7
#define     PICAM1X                 1
#define     PICAM2X                 2

namespace rpi_motioncam {
    class RPiMotionCam {
        
        public:
            static RPiMotionCam* create();
            MMAL_STATUS_T init(int mode, bool preview);
            int getWidth();
            int getHeight();

        private:
            RPiMotionCam();
            ~RPiMotionCam();
            static RPiMotionCam *instance_;

            int width_;
            int height_;
            bool preview_ = false;
            int camera_mode_ = PICAM1X_AUTO;
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

            MMAL_ES_FORMAT_T *format;

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

            static void camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

    };
}

#endif
