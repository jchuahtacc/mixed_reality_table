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
                    err = "No errors";
                    processor = "Aruco Censor";
                }

                ~ArucoCompute() {
                }

                bool process(Mat& image, Ptr< SharedData >& data,  result_t& result) {
                    vector< Marker >::iterator marker = data->markers.begin();
                    for (; marker < markers.end(); marker++) {
                        cv::circle(image, marker->center, marker->radius, black, -1);
                    }
                    return true;
                }

                static Ptr<FrameProcessor> create() {
                    return makePtr<ArucoCensor>().staticCast<FrameProcessor>();
                }

            private:
                Scalar black = Scalar(0, 0, 0);
        };
    }
}

#endif
