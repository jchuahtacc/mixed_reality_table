#include "RegionRecord.h"

using namespace std;
using namespace rpi_motioncam;

namespace mrtable_process {

    RegionRecord::RegionRecord() {
    }

    RegionRecord::RegionRecord(const RegionRecord &other) {
        region = other.region;
        blob_keypoints = other.blob_keypoints;
        aruco_ids = other.aruco_ids;
        corners = other.corners;
        rejected = other.rejected;
        rvecs = other.rvecs;
        tvecs = other.tvecs;
        record_type = other.record_type;
    }

    RegionRecord::RegionRecord(shared_ptr< MotionRegion > region_, string record_type_) : region(region_), record_type(record_type_) {
    }
}
