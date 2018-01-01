#include "rpi_motioncam/callbacks/MotionRegion.h"

namespace rpi_motioncam {
    MotionRegion::MotionRegion() : imgPtr_mtx_p(shared_ptr< tbb::queuing_rw_mutex >(new tbb::queuing_rw_mutex() ) ), id(-1), mandatory(false) {
    }

    MotionRegion::MotionRegion(const MotionRegion& other) {
        row = other.row;
        col = other.col;
        width = other.width;
        height = other.height;
        imgPtr = other.imgPtr;
        roi = other.roi;
        imgPtr_mtx_p = other.imgPtr_mtx_p;
        id = other.id;
        mandatory = other.mandatory;
        tag = other.tag;
        events = other.events;
    }

    void MotionRegion::log_event(string event_name) {
        events.push_back((MotionRegionEvent){ .time = std::chrono::system_clock::now(), .event = event_name });
    }

    bool MotionRegion::get_event(string event_name, std::chrono::time_point<std::chrono::system_clock> &destination) {
        for (auto it = events.begin(); it != events.end(); ++it) {
            MotionRegionEvent event = *it;
            if (event.event == event_name) {
                destination = event.time;
                return true;
            }
        }
        return false;
    }

    void MotionRegion::allocate(cv::Rect roi_) {
        roi = roi_;
        imgPtr = shared_ptr< Mat >( new Mat(roi.height, roi.width, CV_8U) );
    }

}
