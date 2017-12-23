#ifndef __RAWOUTPUTCALLBACK_H__
#define __RAWOUTPUTCALLBACK_H__

#include "RaspiCallback.h"

namespace raspivid {
    class RawOutputCallback : public RaspiCallback {
        void callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
        private:
            int frame_num = 0;
    };
}


#endif /* __RAWOUTPUTCALLBACK_H__ */

