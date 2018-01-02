#include "ArucoFunctor.h"
#include <opencv2/aruco.hpp>
#include <iostream>

using namespace std;
using namespace mrtable::config;

namespace mrtable_process {
    RegionRecord ArucoFunctor::operator()( RegionRecord input ) {
        if (!input.region->tag.empty() && !(input.region->tag == "aruco")) {
            return input;
        }
        //cout << "Aruco " << input.region->id << endl;
        MOTIONREGION_READ_LOCK(input.region);
        aruco::detectMarkers( *(input.region->imgPtr), ServerConfig::dictionary, input.corners, input.aruco_ids, ServerConfig::detectorParameters, input.rejected); 
        aruco::estimatePoseSingleMarkers(input.corners, ServerConfig::markerLength, ServerConfig::cameraMatrix, ServerConfig::distortionCoefficients, input.rvecs, input.tvecs); 
        //detector->detect( *(input.region->imgPtr), input.blob_keypoints);
        return input;
    }
}
