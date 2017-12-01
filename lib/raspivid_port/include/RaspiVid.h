#ifndef _GNU_SOURCE
   #define _GNU_SOURCE
#endif


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
#include "MotionVectorCallback.h"
#include "RawOutputCallback.h"
#include "components/RaspiRenderer.h"
#include "components/RaspiEncoder.h"
#include "components/RaspiSplitter.h"
#include "components/RaspiResize.h"
#include "components/RaspiCamera.h"


// Video format information
// 0 implies variable
#define VIDEO_FRAME_RATE_NUM 30
#define VIDEO_FRAME_RATE_DEN 1

/// Video render needs at least 2 buffers.
#define VIDEO_OUTPUT_BUFFERS_NUM 3

namespace raspivid {
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
    struct RASPIVID_OPTIONS_S
    {
       uint32_t width;                          /// Requested width of image
       uint32_t height;                         /// requested height of image
       uint32_t framerate;                      /// Requested frame rate (fps)
       bool verbose;                        /// !0 if want detailed run information

       bool raw_output;                      /// Output raw video from camera as well
       RAW_OUTPUT_FMT raw_output_fmt;       /// The raw video format
       int cameraNum;                       /// Camera number
       int sensor_mode;			            /// Sensor mode. 0=auto. Check docs/forum for modes selected by other values.

       uint32_t resizer_width;               /// Resizer component width for compressed motion vectors
       uint32_t resizer_height;              /// Resizer component height 
       
       bool preview;

    };

    class RaspiVid {
        public:
            static RaspiVid* create();
            static RaspiVid* create(RASPIVID_OPTIONS_S options);
            static RaspiVid* getInstance();
            static RASPIVID_OPTIONS_S createRaspiVidDefaultOptions();
            MMAL_STATUS_T init();
            MMAL_STATUS_T start();
            void stop();
            ~RaspiVid();

        protected:
            RASPIVID_OPTIONS_S options_;

        private:
            RaspiVid();
            static RaspiVid* singleton_;

            MMAL_STATUS_T create_components();
            MMAL_STATUS_T connect_components();
            MMAL_STATUS_T add_callbacks();

            RaspiCamera *camera;
            RaspiRenderer *preview_renderer;
            RaspiEncoder *encoder;
            RaspiSplitter *splitter;
            RaspiResize *resizer;

            MotionVectorCallback *mvCallback;
            RawOutputCallback *roCallback;
    };
}
