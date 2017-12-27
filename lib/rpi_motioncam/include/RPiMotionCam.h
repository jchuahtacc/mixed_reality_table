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
#define     PICAM_DEFAULT           0
#define     PICAM1X_1920_1080_30    1
#define     PICAM1X_2592_1944_30    2
#define     PICAM1X_1296_972_42     3
#define     PICAM1X_1296_730_49     4
#define     PICAM1X_640_480_60      5
#define     PICAM1X_640_480_90      6
#define     PICAM2X_1920_1080_30    7
#define     PICAM2X_3280_2464_15    8
#define     PICAM2X_1640_1232_40    9
#define     PICAM2X_1640_922_40     10
#define     PICAM2X_1280_720_90     11
#define     PICAM2X_640_480_90      12
#define     PICAM1X                 1
#define     PICAM2X                 2

#define     VIDEO_OUTPUT_BUFFERS_NUM        3
#define     PREVIEW_LAYER                   2
#define     MAX_BITRATE_LEVEL_4             25000000

namespace rpi_motioncam {
    class RPiMotionCam {
        
        public:
            static RPiMotionCam* create();
            MMAL_STATUS_T init();
            MMAL_STATUS_T init(int mode);
            MMAL_STATUS_T init(int mode, bool preview);
            MMAL_STATUS_T start();
            int getWidth();
            int getHeight();
            int getFPS();

        private:
            RPiMotionCam();
            ~RPiMotionCam();
            static RPiMotionCam *instance_;

            uint32_t width_;
            uint32_t height_;
            uint32_t fps_;
            uint32_t camera_mode_;
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
            MMAL_PORT_T *splitter_output_port_0 = NULL;
            MMAL_PORT_T *splitter_output_port_1 = NULL;
            MMAL_PORT_T *splitter_preview_port = NULL;
            MMAL_PORT_T *null_sink_input_port = NULL;
            MMAL_CONNECTION_T *preview_connection = NULL;
            MMAL_CONNECTION_T *splitter_connection = NULL;
            MMAL_CONNECTION_T *encoder_connection = NULL;
            MMAL_POOL_T *encoder_pool = NULL;
            MMAL_POOL_T *splitter_pool = NULL;

            MMAL_ES_FORMAT_T *format;
            MMAL_VIDEO_LEVEL_T h264level_;

            MMAL_STATUS_T create_camera_component();
            MMAL_STATUS_T create_preview_component();
            MMAL_STATUS_T create_splitter_component();
            MMAL_STATUS_T create_encoder_component();
            MMAL_STATUS_T create_null_sink_component();
            MMAL_STATUS_T make_connections();
            MMAL_STATUS_T add_callbacks();
            MMAL_STATUS_T send_buffers();

            MMAL_STATUS_T connect_ports(MMAL_PORT_T *output_port, MMAL_PORT_T *input_port, MMAL_CONNECTION_T **connection);

            void check_disable_port(MMAL_PORT_T *port);
            void destroy_camera_ports();
            void destroy_connections();
            void disable_components();
            void destroy_components();

            bool setCameraMode(int mode);

            static void camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
            static void encoder_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
            static void splitter_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

    };
}

#endif
