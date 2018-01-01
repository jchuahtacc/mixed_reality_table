#include "FramePreview.h"
#include <stdlib.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;

namespace rpi_motioncam {

    void FramePreview::draw(const MotionFrame &frame) {
        if (frame.regions.size()) {
            MMAL_BUFFER_HEADER_T *buffer = renderer_->get_buffer();
            Mat img = Mat::zeros(options_.resizer_height, options_.resizer_width, CV_8UC3);
            for (auto it = frame.regions.begin(); it != frame.regions.end(); ++it) {
                shared_ptr< MotionRegion > region = *it;
                Scalar color = Scalar(255, 0, 0);
                if (region->mandatory) {
                    color = Scalar(0, 0, 255);
                }
                rectangle(img, Point(region->roi.x * width_scale, region->roi.y * height_scale), Point((region->roi.x + region->roi.width) * width_scale, (region->roi.y + region->roi.height) * height_scale), color, 2);
            }
            memcpy(buffer->data, img.data, options_.resizer_width * options_.resizer_height * 3);
            renderer_->send_buffer(buffer);
        }
    }

    FramePreview::FramePreview(RPIMOTIONCAM_OPTION_S options) : options_(options) {
        RASPIOVERLAYRENDERER_FORMAT_S format = RaspiOverlayRenderer::createDefaultOverlayFormat();
        format.width = options.resizer_width;
        format.height = options.resizer_height;
        format.layer = 128;
        format.alpha = 128;
        format.fullscreen = true;
        format.encoding = MMAL_ENCODING_RGB24;
        width_scale = options.resizer_width * 1.0 / options.width;
        height_scale = options.resizer_height * 1.0 / options.height;
        renderer_ = RaspiOverlayRenderer::create(format);
    }

}
