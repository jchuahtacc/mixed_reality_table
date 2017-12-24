#include "RawOutputCallback.h"

namespace raspivid {
    RawOutputCallback::RawOutputCallback() {
        start = std::chrono::system_clock::now();
    }
    void RawOutputCallback::callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
        /*
        buffer_count++;
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
        if (elapsed.count() > 1000) {
            vcos_log_error("RawOutputCallback::callback(): %d buffers in last second", buffer_count);
            buffer_count = 0;
            start = std::chrono::system_clock::now();
        }
        */

        if ((buffer->flags & MMAL_BUFFER_HEADER_FLAG_FRAME_START == MMAL_BUFFER_HEADER_FLAG_FRAME_START) || 
            (buffer->flags & MMAL_BUFFER_HEADER_FLAG_FRAME == MMAL_BUFFER_HEADER_FLAG_FRAME)) {
            int i = 0;
            /*
            if (imgPtr == nullptr) {
                int width = port->format->es->video.width;
                int height = port->format->es->video.height;
                int size = width * height;
                vcos_log_error("Allocating cv::Mat of size %d", size);
                imgPtr = shared_ptr< Mat >(new Mat(width, height, CV_8UC3));
                vcos_log_error("Copying buffer length %d", buffer->length);
                memcpy(imgPtr->data, buffer->data, buffer->length);
                vcos_log_error("Buffer data copied");
            }
            */
        }
    }

    void RawOutputCallback::post_process() {
        if (!written && imgPtr != nullptr) {
            written = true;
            if (cv::imwrite("test_output.jpg", *imgPtr)) {
                vcos_log_error("Wrote test_output.jpg");
            } else {
                vcos_log_error("Error writing test_output");
            }
        }
    }
}
