#ifndef __RASPICAMERA_H__
#define __RASPICAMERA_H__

#include <memory>
#include "components/RaspiComponent.h"
#include "components/RaspiRenderer.h"
#include "RaspiPort.h"
#include "RaspiCamControl.h"

// Standard port setting for the camera component
#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2

// Video format information
// 0 implies variable
#define VIDEO_FRAME_RATE_NUM 30
#define VIDEO_FRAME_RATE_DEN 1

/// Video render needs at least 2 buffers.
#define VIDEO_OUTPUT_BUFFERS_NUM 3


namespace raspivid {

    using namespace raspi_cam_control;

    class RaspiCameraCallback {
        public:
            virtual void callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) =0;
    };

    typedef struct {
        shared_ptr< RaspiCameraCallback > cb_instance;
    } RASPICAMERA_USERDATA_S;

    class DefaultRaspiCameraCallback : public RaspiCameraCallback {
        public:
            void callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
    };

    struct RASPICAMERA_OPTION_S {
        uint32_t height;
        uint32_t width;
        uint32_t framerate;
        int cameraNum;
        int sensor_mode;
        bool verbose;
        RASPICAM_CAMERA_PARAMETERS camera_parameters; 
        shared_ptr< RaspiCameraCallback > settings_callback;
    };

    class RaspiCamera : public RaspiComponent {
        public:
            shared_ptr< RaspiPort > still;
            shared_ptr< RaspiPort > video;
            shared_ptr< RaspiPort > preview;
            static shared_ptr< RaspiCamera > create();
            static shared_ptr< RaspiCamera > create(RASPICAMERA_OPTION_S options);
            static RASPICAMERA_OPTION_S createDefaultCameraOptions();
            void destroy();
            MMAL_STATUS_T start();
        protected:
            const char* component_name();
            MMAL_STATUS_T init();
            RASPICAMERA_OPTION_S options_;
        private:
            static void callback_wrapper(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
            RASPICAMERA_USERDATA_S userdata;
    };
};

#endif /* __RASPICAMERA_H__ */
