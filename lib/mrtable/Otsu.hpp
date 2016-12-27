#ifndef __OTSU_HPP__
#define __OTSU_HPP__

#include "mrtable.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace mrtable::process;
using namespace cv;

namespace mrtable {
    namespace process {
        class Otsu : public FrameProcessor {
            public: 
                Otsu() {
                    err = "No errors";
                    processor = "Otsu";
                }

                ~Otsu() {
                }

                bool process(Mat input, Mat output, result_t& result) {
                    cvtColor(input, output, CV_BGR2GRAY);
                    double* stddev = new double;
                    *stddev = threshold(input, output, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
                    result.outputs[RESULT_KEY_OTSU_STD_DEV] = stddev;
                    return true;
                }
        };
    }
}

#endif
