#include "rpi_motioncam/callbacks/MotionData.h"

namespace rpi_motioncam {
    concurrent_queue< MotionFrame > MotionData::staging_queue = concurrent_queue< MotionFrame >();
    concurrent_queue< MotionFrame > MotionData::ready_queue = concurrent_queue< MotionFrame >();
    vector< shared_ptr< MotionRegion > > MotionData::mandatory_regions = vector< shared_ptr< MotionRegion > >();
    shared_ptr< tbb::mutex > MotionData::mandatory_region_mtx_ptr = shared_ptr< tbb::mutex >( new tbb::mutex() );


    void MotionData::set_mandatory_region( shared_ptr< MotionRegion > region ) {
        region->mandatory = true;
        tbb::mutex::scoped_lock lock(*mandatory_region_mtx_ptr);
        for (auto it = mandatory_regions.begin(); it != mandatory_regions.end(); ++it) {
            if ((*it) == region) {
                return;
            }
        }
        mandatory_regions.push_back(region);
    }

    void MotionData::clear_mandatory_region( shared_ptr< MotionRegion > region ) {
        tbb::mutex::scoped_lock lock(*mandatory_region_mtx_ptr);
        for (auto it = mandatory_regions.begin(); it != mandatory_regions.end(); ++it) {
            if ((*it) == region) {
                mandatory_regions.erase(it);
                return;
            }
        }
    }

    vector< shared_ptr< MotionRegion > > MotionData::get_mandatory_regions() {
        tbb::mutex::scoped_lock lock(*mandatory_region_mtx_ptr);
        vector< shared_ptr< MotionRegion > > result = MotionData::mandatory_regions;
        return result;
    }

    void MotionData::stage_frame( MotionFrame frame ) {
        MotionData::staging_queue.push( frame );
    }

    bool MotionData::get_staged_frame( MotionFrame &destination ) {
        return MotionData::staging_queue.try_pop( destination );
    }

    void MotionData::ready_frame( MotionFrame frame ) {
        frame.ready_timepoint = std::chrono::system_clock::now();
        MotionData::ready_queue.push(frame);
    }

    bool MotionData::get_ready_frame( MotionFrame &destination ) {
        return MotionData::ready_queue.try_pop( destination );
    }

    bool MotionData::has_staged_frames() {
        return !MotionData::staging_queue.empty();
    }

    bool MotionData::has_ready_frames() {
        return !MotionData::ready_queue.empty();
    }
}
