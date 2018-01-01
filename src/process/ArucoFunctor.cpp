#include "ArucoFunctor.h"
#include <opencv2/aruco.hpp>

using namespace std;
using namespace mrtable::config;

namespace mrtable_process {
    RegionRecord ArucoFunctor::operator()( RegionRecord input ) {
        input.region->log_event("aruco_start");
        aruco::detectMarkers( *(input.region->imgPtr), ServerConfig::dictionary, input.corners, input.aruco_ids, ServerConfig::detectorParameters, input.rejected); 
        aruco::estimatePoseSingleMarkers(input.corners, ServerConfig::markerLength, ServerConfig::cameraMatrix, ServerConfig::distortionCoefficients, input.rvecs, input.tvecs); 
        input.region->log_event("aruco_finish");
        return input;
    }
}
