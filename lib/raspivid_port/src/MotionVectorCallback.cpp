#include "MotionVectorCallback.h"

namespace raspivid {

    MotionVectorCallback::MotionVectorCallback(int width, int height) : cols_(width / 16), rows_(height / 16) {
        fresh = false;
    }

    void MotionVectorCallback::callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
        if ((buffer->flags & MMAL_BUFFER_HEADER_FLAG_CODECSIDEINFO)) {
            // vcos_log_error("Motion vector callback buffer length: %d", buffer->length);
            motion = Mat::zeros(rows_, cols_, CV_8U);
            for (int row = 0; row < rows_; row++) {
                for (int col = 0; col < cols_; col++) {
                    int motion_pos = row * cols_ + col;
                    int buffer_pos = (row * (cols_ + 1) + col) * 4;
                    motion.data[ motion_pos ] = buffer->data[buffer_pos + 2];
                    // copy MSB only
                    // motion.data[ motion_pos + 1 ] = buffer->data[ buffer_pos + 3 ];
                }
            }
            fresh = true;
        }
    }

    void MotionVectorCallback::post_process() {
    }
}
