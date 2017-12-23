#include "RawOutputCallback.h"

namespace raspivid {
    void RawOutputCallback::callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
        vcos_log_error("RawOutputCallback::callback(): buffer length %d buffer alloc_size %d", buffer->length, buffer->alloc_size);
    }
}
