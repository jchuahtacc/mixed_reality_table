#include "MotionFrame.h"

namespace rpi_motioncam {

    MotionFrame::MotionFrame() {
    }

    MotionFrame::MotionFrame( vector< shared_ptr< MotionRegion > > regions_ ) : regions(regions_), staged_timepoint(std::chrono::system_clock::now()) {
    }

    MotionFrame::MotionFrame(const MotionFrame &other) {
        regions = other.regions;
        staged_timepoint = other.staged_timepoint;
        ready_timepoint = other.ready_timepoint;
    }


}
