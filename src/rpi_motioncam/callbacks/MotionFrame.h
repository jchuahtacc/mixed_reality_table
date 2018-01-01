#ifndef __MOTIONFRAME_H__
#define __MOTIONFRAME_H__

#include <vector>
#include <memory>
#include <chrono>
#include "MotionRegion.h"

using namespace std;

namespace rpi_motioncam {
    class MotionFrame {
        public:
            MotionFrame();
            MotionFrame(const MotionFrame &other);
            MotionFrame( vector< shared_ptr< MotionRegion > > regions_);
            vector< shared_ptr< MotionRegion > > regions;
            std::chrono::time_point<std::chrono::system_clock> staged_timepoint;
            std::chrono::time_point<std::chrono::system_clock> ready_timepoint; 
    };
}

#endif
