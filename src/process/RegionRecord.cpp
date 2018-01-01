#include "RegionRecord.h"

using namespace std;
using namespace rpi_motioncam;

namespace mrtable_process {

    RegionRecord::RegionRecord() {
    }

    RegionRecord::RegionRecord(const RegionRecord &other) : RegionRecord(other.region) {
    }

    RegionRecord::RegionRecord(shared_ptr< MotionRegion > region_) : region(region_) {
    }
}
