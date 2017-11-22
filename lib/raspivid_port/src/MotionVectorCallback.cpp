#include "MotionVectorCallback.h"

namespace raspivid {
    void MotionVectorCallback::callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
        if ((buffer->flags & MMAL_BUFFER_HEADER_FLAG_CODECSIDEINFO)) {
            vcos_log_error("Motion vector callback buffer length: %d", buffer->length);
        }
    }
}
