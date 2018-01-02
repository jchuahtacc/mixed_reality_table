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
    }

    void MotionRegion::allocate(cv::Rect roi_) {
        roi = roi_;
        imgPtr = shared_ptr< Mat >( new Mat(roi.height, roi.width, CV_8U) );
    }

}
