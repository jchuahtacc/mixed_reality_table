#include "InputFunctor.h"

namespace mrtable_process {
    void InputFunctor::operator()(const shared_ptr< MotionRegion > &region, InputNodeType::output_ports_type &ports) {
        if (region->tag.empty() || region->tag == "aruco") {
            std::get< ARUCO_PORT >(ports).try_put( RegionRecord( region ) );
        }
        if (region->tag.empty() || region->tag == "aruco") {
            std::get< BLOB_PORT >(ports).try_put( RegionRecord( region ) );
        }
    }
}
