#ifndef __MOTIONVECTORCALLBACK_H__
#define __MOTIONVECTORCALLBACK_H__

#include "RaspiCallback.h"

namespace raspivid {
    class MotionVectorCallback : public RaspiCallback {
        void callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
    };
}


#endif /* __MOTIONVECTORCALLBACK_H__ */

