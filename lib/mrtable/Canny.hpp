#ifndef __CANNY_HPP__
#define __CANNY_HPP__

#include "keydefs.hpp"
#include "FrameProcessor.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace mrtable::process;
using namespace cv;

namespace mrtable {
    namespace process {
        class Canny : public FrameProcessor {
            public: 
                Canny() {
                    err = "No RESULT_KEY_OTSU_STD_DEV value";
                    processor = "Canny";
                }

                ~Canny() {
                }

                bool process(Mat& image, result_t& result) {
                    double* ptr = static_cast<double*>(result.outputs[RESULT_KEY_OTSU_STD_DEV]);
                    if (ptr != NULL) {
                        double stddev = *ptr;
                        cv::Canny(image, image, stddev * 0.5, stddev);
                        return true;
                    } else {
                        cv::Canny(image, image, 100, 300);
                        return true;
                    }
                }

                static Ptr<FrameProcessor> create() {
                    return makePtr<Canny>().staticCast<FrameProcessor>();
                }
        };
    }
}

#endif
