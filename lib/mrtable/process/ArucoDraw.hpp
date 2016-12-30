#ifndef __ARUCOCOMPUTE_HPP__
#define __ARUCOCOMPUTE_HPP__

#include "keydefs.hpp"
#include "Marker.hpp"
#include "FrameProcessor.hpp"
#include <tuio/TuioServer.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <bitset>

using namespace mrtable::data;
using namespace mrtable::process;
using namespace cv;
using namespace cv::aruco;
using namespace std;

namespace mrtable {
    namespace process {
        class ArucoCompute : public FrameProcessor {
            public: 
                ArucoCompute() {
                    corners = SharedData::getPtr< vector< vector< Point2f > > >(RESULT_KEY_ARUCO_CORNERS);
                    ids = SharedData::getPtr< vector< int > >(RESULT_KEY_ARUCO_IDS);
                    err = "No errors";
                    processor = "Aruco Draw";
                }

                ~ArucoCompute() {
                }

                bool process(Mat& image, result_t& result) {
                    cv::aruco::drawDetectedMarkers(image, *corners, *ids);
                    return true;
                }

                static Ptr<FrameProcessor> create() {
                    return makePtr<ArucoDraw>().staticCast<FrameProcessor>();
                }

            private:
                vector< vector< Point2f > > *corners;
                vector< int > *ids;
        };
    }
}

#endif
