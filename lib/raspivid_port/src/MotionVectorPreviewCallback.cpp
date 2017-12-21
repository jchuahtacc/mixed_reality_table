#include "MotionVectorPreviewCallback.h"
#include <stdlib.h>

namespace raspivid {
    void MotionVectorPreviewCallback::post_process() {
        if (fresh) {
            MMAL_BUFFER_HEADER_T *buffer = renderer_->get_buffer();
            int row_offset = cols_ * 16 * 3; 
            for (int row = 0; row < rows_; row++) {
                for (int col = 0; col < cols_; col++) {
                    int data_pos = (row * cols_ + col);
                    int buffer_pos = (row * cols_ * 256 + col * 16) * 3;
                    for (int i = 0; i < 16; i++) {
                        memset(&buffer->data[buffer_pos + i * row_offset],  motion.data[data_pos], 48);
                    }
                    /*
                    memcpy(&buffer->data[buffer_pos]
                    buffer->data[buffer_pos] = motion.data[data_pos];
                    buffer->data[buffer_pos + 1] = motion.data[data_pos];
                    buffer->data[buffer_pos + 2] = motion.data[data_pos];
                    */
                }
            }
            renderer_->send_buffer(buffer);
            fresh = false;
        }
    }

    MotionVectorPreviewCallback::MotionVectorPreviewCallback(int width, int height) : MotionVectorCallback(width, height) {
        RASPIOVERLAYRENDERER_FORMAT_S format = RaspiOverlayRenderer::createDefaultOverlayFormat();
        format.width = width;
        format.height = height;
        format.layer = 128;
        format.alpha = 200;
        format.fullscreen = true;
        format.encoding = MMAL_ENCODING_RGB24;
        renderer_ = RaspiOverlayRenderer::create(format);
    }

}
