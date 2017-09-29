#include "RawOutputCallback.h"

namespace raspivid {
    void RawOutputCallback::callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
        vcos_log_error("Raw output callback");
    }
}
