#include "MotionRegionPreviewCallback.h"
#include <stdlib.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;

namespace rpi_motioncam {

    int MotionRegionPreviewCallback::buffer_position(int row, int col, int row_offset = 0) {
        return (row * cols_ * 256 + col * 16) * 3 + row_offset * cols_ * 48;
    }

    void MotionRegionPreviewCallback::post_process() {
        if (lastFrame.regions.size()) {
            MMAL_BUFFER_HEADER_T *buffer = renderer_->get_buffer();
            Mat img = Mat::zeros(options_.resizer_height, options_.resizer_width, CV_8UC3);
            for (auto it = lastFrame.regions.begin(); it != lastFrame.regions.end(); ++it) {
                shared_ptr< MotionRegion > region = *it;
                rectangle(img, Point(region->col * 16, region->row * 16), Point((region->col + region->width) * 16, (region->row + region->height) * 16), Scalar(255, 0, 0), 2);
            }
            memcpy(buffer->data, img.data, options_.resizer_width * options_.resizer_height * 3);
            renderer_->send_buffer(buffer);
        }
    }

    MotionRegionPreviewCallback::MotionRegionPreviewCallback(RPIMOTIONCAM_OPTION_S options) : MotionVectorCallback(options) {
        RASPIOVERLAYRENDERER_FORMAT_S format = RaspiOverlayRenderer::createDefaultOverlayFormat();
        format.width = options.resizer_width;
        format.height = options.resizer_height;
        format.layer = 128;
        format.alpha = 200;
        format.fullscreen = true;
        format.encoding = MMAL_ENCODING_RGB24;
        renderer_ = RaspiOverlayRenderer::create(format);
    }

}
