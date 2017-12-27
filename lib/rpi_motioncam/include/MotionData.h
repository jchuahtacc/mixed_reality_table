#ifndef __MOTIONDATA_H__
#define __MOTIONDATA_H__

#include "MotionRegion.h"
#include "tbb/tbb.h" 
#include "tbb/concurrent_queue.h"
#include <opencv2/core.hpp>
#include <memory>
#include <chrono>
#include <vector>

using namespace std;
using namespace tbb;

namespace rpi_motioncam {
    class MotionData {
        public:
            shared_ptr< vector< MotionRegion > > regions;
            std::chrono::time_point<std::chrono::system_clock> region_timepoint;
            std::chrono::time_point<std::chrono::system_clock> frame_timepoint; 

            static void stage_regions( shared_ptr< vector< MotionRegion > > regions );
            static bool get_staged_regions( shared_ptr< MotionData >& destination );
            static void stage_ready_frame( shared_ptr< MotionData > frame );
            static bool get_ready_frame( shared_ptr< MotionData >& destination ); 
            static bool has_staged_regions();
            static bool has_ready_frame();

        protected:
            MotionData( shared_ptr< vector< MotionRegion > > regions_ );
            static concurrent_queue< shared_ptr< MotionData > > staging_queue;
            static concurrent_queue< shared_ptr< MotionData > > ready_queue;

    };
}


#endif /* __MOTIONDATA_H__ */
