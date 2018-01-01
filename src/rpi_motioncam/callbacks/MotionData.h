#ifndef __MOTIONDATA_H__
#define __MOTIONDATA_H__

#include "MotionRegion.h"
#include "MotionFrame.h"
#include "tbb/tbb.h" 
#include "tbb/concurrent_queue.h"
#include "tbb/mutex.h"
#include <opencv2/core.hpp>
#include <memory>
#include <chrono>
#include <vector>

using namespace std;
using namespace tbb;

namespace rpi_motioncam {
    class MotionData {
        public:
            
            static void stage_frame( vector< shared_ptr< MotionRegion > > regions );
            static bool get_staged_frame( MotionFrame &destination );
            static void ready_frame( MotionFrame frame );
            static bool get_ready_frame( MotionFrame &destination ); 
            static bool has_staged_frames();
            static bool has_ready_frames();
            static void set_mandatory_region( shared_ptr< MotionRegion > region );
            static void clear_mandatory_region( shared_ptr< MotionRegion > region );
            static vector< shared_ptr< MotionRegion > > get_mandatory_regions();

        protected:
            static concurrent_queue< MotionFrame > staging_queue;
            static concurrent_queue< MotionFrame > ready_queue;
            static vector< shared_ptr< MotionRegion > > mandatory_regions;
            static shared_ptr< tbb::mutex >  mandatory_region_mtx_ptr;

    };
}


#endif /* __MOTIONDATA_H__ */
