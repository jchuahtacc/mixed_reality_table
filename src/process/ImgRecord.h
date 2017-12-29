#ifndef __IMGRECORD_H__
#define __IMGRECORD_H__

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
    typedef struct {
        std::chrono::time_point<std::chrono::system_clock> time;
        string event;
    } ImgRecordEvent;

    class ImgRecord {
        public:
            ImgRecord(shared_ptr< MotionData > motion_data_);
            shared_ptr< MotionData > motion_data;
            vector< ImgRecordEvent > events;
            vector< KeyPoint > blob_keypoints;

            vector< int > aruco_ids;
            vector< vector< Point2f > > corners, rejected;
            vector< Vec3d > rvecs, tvecs;

            bool blob_detector_finished = false;
            bool aruco_detector_finished = false;
    };
}

#endif

