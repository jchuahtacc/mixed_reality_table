#include "RawOutputCallback.h"

namespace raspivid {
    RawOutputCallback::RawOutputCallback(int width, int height) : width_(VCOS_ALIGN_UP(width, 32)), height_(VCOS_ALIGN_UP(height, 16)) {
        start = std::chrono::system_clock::now();
        size_ = width_ * height_;
        imgPtr = shared_ptr< Mat >( new Mat(height_, width_, CV_8U) );
    }

    void RawOutputCallback::copy_buffer(MMAL_BUFFER_HEADER_T *buffer) {
        memcpy(imgPtr->data, buffer->data, size_);
    }

    void RawOutputCallback::save_buffer_copy(string filename) {
        cv::imwrite(filename.c_str(), *imgPtr);
    }

    void RawOutputCallback::callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
            imgPtr = shared_ptr< Mat >( new Mat(height_, width_, CV_8U ) );
            //copy_buffer(buffer);
            buffer_count++;
            //vcos_log_error("RawOutputCallback::callback(): buffer #%d", buffer_count);
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
            if (elapsed.count() > 1000) {
                vcos_log_error("RawOutputCallback::callback(): %d buffers in last second", buffer_count);
                buffer_count = 0;
                start = std::chrono::system_clock::now();
            }
    }

    void RawOutputCallback::post_process() {
    }
}
