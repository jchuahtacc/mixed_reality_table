#ifndef __ARUCOCENSOR_HPP__
#define __ARUCOCENSOR_HPP__

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
                    markers = SharedData::getPtr< vector< Marker > >(RESULT_KEY_ARUCO_MARKERS);
                    err = "No errors";
                    processor = "Aruco Censor";
                }

                ~ArucoCompute() {
                }

                bool process(Mat& image, result_t& result) {
                    vector< Marker >::iterator  marker = markers->begin();
                    for (; marker < markers->end(); marker++) {
                        cv::circle(image, marker->center, marker->radius, black, -1);
                    }
                    return true;
                }

                static Ptr<FrameProcessor> create() {
                    return makePtr<ArucoCensor>().staticCast<FrameProcessor>();
                }

            private:
                vector< Marker > *markers;
                Scalar black = Scalar(0, 0, 0);
        };
    }
}

#endif
