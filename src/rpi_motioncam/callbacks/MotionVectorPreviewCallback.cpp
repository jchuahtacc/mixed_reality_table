#include "rpi_motioncam/callbacks/MotionVectorPreviewCallback.h"
#include <stdlib.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace std;
using namespace cv;

namespace rpi_motioncam {

    int MotionVectorPreviewCallback::render_buffer_position(int row, int col, int row_offset = 0) {
        return row_bytes * row + col_bytes * col + row_offset * scanline_bytes;
    }

    void MotionVectorPreviewCallback::post_process() {
        MMAL_BUFFER_HEADER_T *buffer = renderer_->get_buffer();
        memset(buffer->data, 0, options_.resizer_width * options_.resizer_height * 3);
        for (int row = 0; row < rows_; row++) {
            for (int col = 0; col < cols_; col++) {
                for (int offset = 0; offset < 16; offset++) {
                    memset(&buffer->data[render_buffer_position(row, col, offset)], lastBuffer[row * cols_ + col], col_bytes);
                }
            }
        }
        renderer_->send_buffer(buffer);
    }

    MotionVectorPreviewCallback::MotionVectorPreviewCallback(RPIMOTIONCAM_OPTION_S options) : MotionVectorCallback(options) {
        RASPIOVERLAYRENDERER_FORMAT_S format = RaspiOverlayRenderer::createDefaultOverlayFormat();
        format.width = options.resizer_width;
        format.height = options.resizer_height;
        format.layer = 128;
        format.alpha = 128;
        format.fullscreen = true;
        format.encoding = MMAL_ENCODING_RGB24;
        scanline_bytes = options_.resizer_width * 3;
        row_bytes = 16 * scanline_bytes;
        col_bytes = options_.resizer_width / cols_ * 3;
        renderer_ = RaspiOverlayRenderer::create(format);
    }

}
