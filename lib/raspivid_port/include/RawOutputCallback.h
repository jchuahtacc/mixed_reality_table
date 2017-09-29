#ifndef __RAWOUTPUTCALLBACK_H__
#define __RAWOUTPUTCALLBACK_H__

#include "RaspiCallback.h"

namespace raspivid {
    class RawOutputCallback : public RaspiCallback {
        void callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
    };
}


#endif /* __RAWOUTPUTCALLBACK_H__ */

