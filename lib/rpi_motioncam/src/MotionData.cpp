#include "MotionData.h"

namespace rpi_motioncam {
    concurrent_queue< shared_ptr< MotionData > > MotionData::staging_queue = concurrent_queue< shared_ptr< MotionData > >();
    concurrent_queue< shared_ptr< MotionData > > MotionData::ready_queue = concurrent_queue< shared_ptr< MotionData > >();

    MotionData::MotionData( shared_ptr< vector< MotionRegion > > regions_ ) : regions(regions_), region_timepoint(std::chrono::system_clock::now()) {
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
