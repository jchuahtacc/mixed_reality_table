#include "MotionVectorCallback.h"

namespace raspivid {
    void MotionVectorCallback::callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
        vcos_log_error("Motion vector callback");
    }
}
