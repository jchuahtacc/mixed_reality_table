#ifndef __REGIONRECORD_H__
#define __REGIONRECORD_H__

#include "rpi_motioncam/RPiMotionCam.h"
#include <opencv2/features2d.hpp>
#include <opencv2/core.hpp>
#include <memory>
#include <chrono>
#include <string>
#include <vector>

using namespace rpi_motioncam;
using namespace std;

namespace mrtable_process {
    class RegionRecord {
        public:
            RegionRecord();
            RegionRecord(const RegionRecord &other);
            RegionRecord(shared_ptr< MotionRegion > region_, string record_type_);

            shared_ptr< MotionRegion > region;
            vector< KeyPoint > blob_keypoints;

            vector< int > aruco_ids;
            vector< vector< Point2f > > corners, rejected;
            vector< Vec3d > rvecs, tvecs;

            string record_type;
    };
}

#endif

