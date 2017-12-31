#include "rpi_motioncam/callbacks/RawOutputCallback.h"
#include "tbb/queuing_rw_mutex.h"

namespace rpi_motioncam {
    RawOutputCallback::RawOutputCallback(int width, int height) : width_(VCOS_ALIGN_UP(width, 32)), height_(VCOS_ALIGN_UP(height, 16)) {
        start = std::chrono::system_clock::now();
        size_ = width_ * height_;
            }

    void RawOutputCallback::copy_buffer(MMAL_BUFFER_HEADER_T *buffer) {
        memcpy(imgPtr->data, buffer->data, size_);
    }

    void RawOutputCallback::save_buffer_copy(string filename) {
        cv::imwrite(filename.c_str(), *imgPtr);
    }

    void RawOutputCallback::callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
        shared_ptr< MotionData > frame = nullptr;
        shared_ptr< MotionData > candidate = nullptr;
        // If there is a staged region, keep trying to pull it from the staged region queue until we succeed
        // (contention on the queue may cause get_staged_regions to return false)
        while (MotionData::has_staged_regions() && !frame) {
            if (MotionData::get_staged_regions( candidate )) {
                if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - candidate->region_timepoint).count() < REGION_TIMEOUT) {
                    frame = candidate;
                }
            }
        }
        if (frame) {
            auto buffImg = shared_ptr< Mat >(new Mat(height_, width_, CV_8U, buffer->data) );
            // vcos_log_error("RawOutputCallback::callback(): found frame with %d regions", frame->regions->size());
            for (auto region = frame->regions->begin(); region != frame->regions->end(); ++region) {
                MOTIONREGION_WRITE_LOCK(region);
                (*buffImg)( region->roi ).copyTo( *region->imgPtr );
            }
            MotionData::stage_ready_frame( frame );
        }

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
