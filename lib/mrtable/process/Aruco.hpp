#ifndef __ARUCO_HPP__
#define __ARUCO_HPP__

#include "keydefs.hpp"
#include "FrameProcessor.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace mrtable::process;
using namespace cv;
using namespace cv::aruco;

namespace mrtable {
    namespace process {
        class Aruco : public FrameProcessor {
            public: 
                vector< int > ids;
                vector< vector< Point2f > > corners, rejected;
                vector< Vec3d > rvecs, tvecs;
                
                Aruco() {
                    err = "No errors";
                    processor = "Aruco";
                }

                ~Aruco() {
                }

                bool process(Mat& image, result_t& result) {
                    ids.clear();
                    corners.clear();
                    rejected.clear();
                    rvecs.clear();
                    tvecs.clear();
                    double* stddev = new double;
                    *stddev = threshold(image, image, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
                    *stddev = 1.0;
                    result.outputs[RESULT_KEY_OTSU_STD_DEV] = stddev;
                    return true;
                }

                static Ptr<FrameProcessor> create() {
                    return makePtr<Otsu>().staticCast<FrameProcessor>();
                }
        };
    }
}

#endif
