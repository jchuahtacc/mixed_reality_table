
#ifndef __RPIMOTIONCAM_H__
#define __RPIMOTIONCAM_H__
#include <memory>

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
#include "rpi_motioncam/callbacks/MotionRegion.h"
#include "rpi_motioncam/callbacks/MotionData.h"
#include "rpi_motioncam/callbacks/MotionVectorCallback.h"
#include "rpi_motioncam/callbacks/MotionVectorPreviewCallback.h"
#include "rpi_motioncam/callbacks/RawOutputCallback.h"
#include "rpi_motioncam/components/RaspiRenderer.h"
#include "rpi_motioncam/components/RaspiEncoder.h"
#include "rpi_motioncam/components/RaspiSplitter.h"
#include "rpi_motioncam/components/RaspiResize.h"
#include "rpi_motioncam/components/RaspiCamera.h"
#include "rpi_motioncam/components/RaspiNullsink.h"


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

    // Forward
    typedef struct RASPIVID_STATE_S RASPIVID_STATE;

    /** Possible raw output formats
     */
    typedef enum {
       RAW_OUTPUT_FMT_YUV = 1,
       RAW_OUTPUT_FMT_RGB,
       RAW_OUTPUT_FMT_GRAY,
    } RAW_OUTPUT_FMT;

    /** Structure containing all state information for the current run
     */
    struct RPIMOTIONCAM_OPTION_S
    {
       uint32_t width;                          /// Requested width of image
       uint32_t height;                         /// requested height of image
       uint32_t framerate;                      /// Requested frame rate (fps)
       bool verbose;                        /// !0 if want detailed run information

       RAW_OUTPUT_FMT raw_output_fmt;       /// The raw video format
       int cameraNum;                       /// Camera number
       int sensor_mode;			            /// Sensor mode. 0=auto. Check docs/forum for modes selected by other values.

       uint32_t resizer_width;               /// Resizer component width for compressed motion vectors
       uint32_t resizer_height;              /// Resizer component height 
       int motion_threshold;
       
       bool preview;

    };

    class RPiMotionCam {
        public:
            static shared_ptr< RPiMotionCam > create();
            static shared_ptr< RPiMotionCam > create(RPIMOTIONCAM_OPTION_S options);
            static RPIMOTIONCAM_OPTION_S createMotionCamDefaultOptions();
            MMAL_STATUS_T init();
            MMAL_STATUS_T start();
            bool frame_ready();
            shared_ptr< MotionData > get_frame();
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

            shared_ptr< MotionVectorCallback > mvCallback;
            shared_ptr< RawOutputCallback > roCallback;

        };
}

#endif /* __RPIMOTIONCAM_H__ */
