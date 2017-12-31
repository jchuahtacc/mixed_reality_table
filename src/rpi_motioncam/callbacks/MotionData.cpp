#include "rpi_motioncam/callbacks/MotionData.h"

namespace rpi_motioncam {
    concurrent_queue< shared_ptr< MotionData > > MotionData::staging_queue = concurrent_queue< shared_ptr< MotionData > >();
    concurrent_queue< shared_ptr< MotionData > > MotionData::ready_queue = concurrent_queue< shared_ptr< MotionData > >();
    vector< shared_ptr< MotionRegion > > MotionData::mandatory_regions = vector< shared_ptr< MotionRegion > >();
    shared_ptr< tbb::mutex > MotionData::mandatory_region_mtx_ptr = shared_ptr< tbb::mutex >( new tbb::mutex() );

    MotionData::MotionData( shared_ptr< vector< MotionRegion > > regions_ ) : regions(regions_), region_timepoint(std::chrono::system_clock::now()) {
    }

    void MotionData::set_mandatory_region( shared_ptr< MotionRegion > region ) {
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

    void MotionData::stage_regions( shared_ptr< vector< MotionRegion > > regions ) {
        MotionData::staging_queue.push(shared_ptr< MotionData >( new MotionData(regions) ) );
    }

    bool MotionData::get_staged_regions( shared_ptr< MotionData >& destination ) {
        return MotionData::staging_queue.try_pop( destination );
    }

    void MotionData::stage_ready_frame( shared_ptr< MotionData > frame ) {
        frame->frame_timepoint = std::chrono::system_clock::now();
        MotionData::ready_queue.push(frame);
    }

    bool MotionData::get_ready_frame( shared_ptr< MotionData >& destination ) {
        return MotionData::ready_queue.try_pop( destination );
    }

    bool MotionData::has_staged_regions() {
        return !MotionData::staging_queue.empty();
    }

    bool MotionData::has_ready_frame() {
        return !MotionData::ready_queue.empty();
    }
}
