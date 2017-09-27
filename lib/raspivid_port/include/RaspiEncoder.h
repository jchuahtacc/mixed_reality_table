#ifndef __RASPIENCODER_H__
#define __RASPIENCODER_H__

#include "RaspiComponent.h"


#define VIDEO_FRAME_RATE_NUM 30
#define VIDEO_FRAME_RATE_DEN 1
#define MAX_BITRATE_MJPEG 25000000
#define MAX_BITRATE_LEVEL4 25000000
#define MAX_BITRATE_LEVEL42 62500000

namespace raspivid {
      
    struct RASPIENCODER_OPTION_S {
        MMAL_FOURCC_T encoding;
        int bitrate;
        uint32_t width;
        uint32_t height;
        uint32_t framerate;
        uint32_t intraperiod;
        uint32_t quantisationParameter;
        int bInlineHeaders;
        int immutableInput;
        MMAL_VIDEO_PROFILE_T profile;
        MMAL_VIDEO_LEVEL_T level;
        MMAL_VIDEO_INTRA_REFRESH_T intra_refresh_type;
        int inlineMotionVectors;
    };

    class RaspiEncoder : RaspiComponent {
        public:
            static RASPIENCODER_OPTION_S createDefaultEncoderOptions();
            static RaspiEncoder* create(RASPIENCODER_OPTION_S options);
            static RaspiEncoder* create();
            MMAL_PORT_T *input;
            MMAL_PORT_T *output;
            void destroy();
        protected:
            static void callback(MMAL_PORT_T* port, MMAL_BUFFER_HEADER_T* buffer);
            const char* component_name();
            static MMAL_POOL_T* pool;
            MMAL_STATUS_T init();
            RASPIENCODER_OPTION_S options_;
    };
}

#endif /* __RASPIENCODER_H__ */
