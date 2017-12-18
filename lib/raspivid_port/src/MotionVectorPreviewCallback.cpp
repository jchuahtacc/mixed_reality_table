#include "MotionVectorPreviewCallback.h"
#include <stdlib.h>

namespace raspivid {
    void MotionVectorCallback::process(MMAL_BUFFER_HEADER_T *buffer) {
    }

    MotionVectorCallback::MotionVectorCallback(uint16_t width, uint16_t height, RaspiOverlayRenderer *renderer) : width_(width), height_(height), renderer_(renderer)  {
    }

    MotionVectorCallback::~MotionVectorCallback() {
    }
}
