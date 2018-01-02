
#ifndef __RPIMOTIONCAM_H__
#define __RPIMOTIONCAM_H__
#include <memory>

#include "typedefs.h"
#include "bcm_host.h"
#include "interface/vcos/vcos.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"
#include "interface/mmal/mmal_parameters_camera.h"

#include "RaspiCamControl.h"
#include "RaspiPort.h"
#include "callbacks/MotionRegion.h"
#include "callbacks/MotionData.h"
#include "callbacks/MotionVectorCallback.h"
#include "callbacks/RawOutputCallback.h"
#include "components/RaspiRenderer.h"
#include "components/RaspiEncoder.h"
#include "components/RaspiSplitter.h"
#include "components/RaspiResize.h"
#include "components/RaspiCamera.h"
#include "components/RaspiNullsink.h"


// Video format information
// 0 implies variable
#define VIDEO_FRAME_RATE_NUM 30
#define VIDEO_FRAME_RATE_DEN 1

/// Video render needs at least 2 buffers.
#define VIDEO_OUTPUT_BUFFERS_NUM 3

namespace rpi_motioncam {
    using namespace raspi_cam_control;

    // Max bitrate we allow for recording
    const int MAX_BITRATE_MJPEG = 25000000; // 25Mbits/s
    const int MAX_BITRATE_LEVEL4 = 25000000; // 25Mbits/s
    const int MAX_BITRATE_LEVEL42 = 62500000; // 62.5Mbits/s

    /// Interval at which we check for an failure abort during capture
    const int ABORT_INTERVAL = 100; // ms

    int mmal_status_to_int(MMAL_STATUS_T status);
    static void signal_handler(int signal_number);


    class RPiMotionCam {
        public:
            static shared_ptr< RPiMotionCam > create();
            static shared_ptr< RPiMotionCam > create(RPIMOTIONCAM_OPTION_S options);
            static RPIMOTIONCAM_OPTION_S createMotionCamDefaultOptions();
            MMAL_STATUS_T init();
            MMAL_STATUS_T start();
            void stop();
            ~RPiMotionCam();

        protected:
            RPIMOTIONCAM_OPTION_S options_;

        private:
            RPiMotionCam();

            MMAL_STATUS_T create_components();
            MMAL_STATUS_T connect_components();
            MMAL_STATUS_T add_callbacks();

            shared_ptr< RaspiCamera > camera;
            shared_ptr< RaspiRenderer > preview_renderer;
            shared_ptr< RaspiEncoder > encoder;
            shared_ptr< RaspiSplitter > splitter;
            shared_ptr< RaspiResize > resizer;
            shared_ptr< RaspiNullsink > nullsink;
            shared_ptr< RaspiNullsink > nullsink2;

            shared_ptr< MotionVectorCallback > mvCallback;
            shared_ptr< RawOutputCallback > roCallback;

        };
}

#endif /* __RPIMOTIONCAM_H__ */
