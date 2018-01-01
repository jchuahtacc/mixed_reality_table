#include "VectorRenderer.h"
#include <stdlib.h>
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

    static shared_ptr< VectorRenderer > create(RPI_MOTIONCAM_OPTION_S options) {
        RASPIOVERLAYRENDERER_FORMAT_S format = RaspiOverlayRenderer::createDefaultOverlayFormat();
        format.width = options.resizer_width;
        format.height = options.resizer_height;
        format.layer = 128;
        format.alpha = 128;
        format.fullscreen = true;
        format.encoding = MMAL_ENCODING_RGB24;
        return shared_ptr< VectorRenderer >(new VectorRenderer(format, options) );

   }

    VectorRenderer::VectorRenderer(RASPIOVERLAYRENDERER_OPTION_S renderer_options, RPIMOTIONCAM_OPTION_S motioncam_options) : options_(options) {
        scanline_bytes = options_.resizer_width * 3;
        row_bytes = 16 * scanline_bytes;
        col_bytes = options_.resizer_width / cols_ * 3;
        renderer_ = RaspiOverlayRenderer::create(format);
    }

}
