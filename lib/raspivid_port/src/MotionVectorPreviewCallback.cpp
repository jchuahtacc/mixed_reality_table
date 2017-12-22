#include "MotionVectorPreviewCallback.h"
#include <stdlib.h>
#include <opencv2/core.hpp>

namespace raspivid {
    void MotionVectorPreviewCallback::post_process() {
        char *red = new char[48];
        for (int i = 0; i < 16; i++) {
            red[i * 3] = 255;
            red[i * 3 + 1] = 255;
            red[i * 3 + 2] = 255;
        }

        int row_offset = cols_ * 48;
        if (new_vectors) {
            //vcos_log_error("MotionVectorPreviewCallback::post_process(): %d regions", regions.size());
            MMAL_BUFFER_HEADER_T *buffer = renderer_->get_buffer();
            for (auto region = regions.begin(); region != regions.end(); ++region) {
                for (int row = region->row; row < region->row + region->height; row++) {
                    for (int col = region->col; col < region->col + region->width; col++) {
                        int buffer_pos = (row * cols_ * 256 + col * 16) * 3;
                        for (int i = 0; i < 16; i++) {
                             memcpy(&buffer->data[buffer_pos + i * row_offset], red, 48);
                        }
                    }
                }
            }
            /*
            // draw motion vector blocks
            int row_offset = cols_ * 16 * 3; 
            for (int row = 0; row < rows_; row++) {
                for (int col = 0; col < cols_; col++) {
                    int data_pos = (row * cols_ + col);
                    int buffer_pos = (row * cols_ * 256 + col * 16) * 3;
                    for (int i = 0; i < 16; i++) {
                        memset(&buffer->data[buffer_pos + i * row_offset],  motion.data[data_pos], 48);
                    }
                }
            }
            */
            delete red;
            renderer_->send_buffer(buffer);
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
