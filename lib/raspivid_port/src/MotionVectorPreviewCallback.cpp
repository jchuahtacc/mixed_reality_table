#include "MotionVectorPreviewCallback.h"
#include <stdlib.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;

namespace raspivid {

    int MotionVectorPreviewCallback::buffer_position(int row, int col, int row_offset = 0) {
        return (row * cols_ * 256 + col * 16) * 3 + row_offset * cols_ * 48;
    }

    void MotionVectorPreviewCallback::post_process() {
        if (lastRegions) {
            MMAL_BUFFER_HEADER_T *buffer = renderer_->get_buffer();
            Mat img = Mat::zeros(480, 640, CV_8UC3);
            for (auto region = lastRegions->begin(); region != lastRegions->end(); ++region) {
                rectangle(img, Point(region->col * 16, region->row * 16), Point((region->col + region->width) * 16, (region->row + region->height) * 16), Scalar(255, 0, 0), 2);
            }
            memcpy(buffer->data, img.data, 480 * 640 * 3);
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
