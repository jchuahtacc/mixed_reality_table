#ifndef __CONTOUR_HPP__
#define __CONTOUR_HPP__

#include "keydefs.hpp"
#include "Marker.hpp"
#include "FrameProcessor.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <bitset>

using namespace mrtable::process;
using namespace cv;
using namespace cv::aruco;
using namespace std;

namespace mrtable {
    namespace process {
        class Contour : public FrameProcessor {
            public: 
                vector< vector<Point> > contours;
                vector<Vec4i> hierarchy;

                Contour() {
                    err = "No errors";
                    processor = "Contour";
                }

                ~Contour() {
                    outputs->erase(RESULT_KEY_CONTOUR_CONTOURS);
                    outputs->erase(RESULT_KEY_CONTOUR_HIERARCHY);
                }

                void init(Ptr<ServerConfig> config) {
                    outputs->put(RESULT_KEY_CONTOUR_CONTOURS, &contours);
                    outputs->put(RESULT_KEY_CONTOUR_HIERARCHY, &hierarchy);
                }

                bool process(Mat& image, result_t& result) {
                    findContours(image, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE);
                    return true;
                }

                static Ptr<FrameProcessor> create() {
                    return makePtr<Contour>().staticCast<FrameProcessor>();
                }
        };
    }
}

#endif
