#include "VectorPreview.h"
#include <stdlib.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace std;
using namespace cv;

namespace rpi_motioncam {

    int VectorPreview::render_buffer_position(int row, int col, int row_offset = 0) {
        return row_bytes * row + col_bytes * col + row_offset * scanline_bytes;
    }

    void VectorPreview::draw(char* vectorBuffer) {
        MMAL_BUFFER_HEADER_T *buffer = renderer_->get_buffer();
        memset(buffer->data, 0, options_.resizer_width * options_.resizer_height * 3);
        for (int row = 0; row < rows_; row++) {
            for (int col = 0; col < cols_; col++) {
                for (int offset = 0; offset < 16; offset++) {
                    memset(&buffer->data[render_buffer_position(row, col, offset)], vectorBuffer[row * cols_ + col], col_bytes);
                }
            }
        }
        renderer_->send_buffer(buffer);
    }

    VectorPreview::VectorPreview(RPIMOTIONCAM_OPTION_S options) : options_(options) {
        RASPIOVERLAYRENDERER_FORMAT_S format = RaspiOverlayRenderer::createDefaultOverlayFormat();
        format.width = options.resizer_width;
        format.height = options.resizer_height;
        format.layer = 100;
        format.alpha = 64;
        format.fullscreen = true;
        format.encoding = MMAL_ENCODING_RGB24;
        cols_ = options_.resizer_width / 16;
        rows_ = options_.resizer_height / 16;
        scanline_bytes = options_.resizer_width * 3;
        row_bytes = 16 * scanline_bytes;
        col_bytes = options_.resizer_width / cols_ * 3;
        renderer_ = RaspiOverlayRenderer::create(format);
    }

}
