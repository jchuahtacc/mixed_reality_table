#ifndef __OTSUCALC_HPP__
#define __OTSUCALC_HPP__

#include "keydefs.hpp"
#include "FrameProcessor.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace mrtable::process;
using namespace cv;

namespace mrtable {
    namespace process {
        class OtsuCalc : public FrameProcessor {
            public: 
                Mat discard;
                OtsuCalc() {
                    err = "No errors";
                    processor = "Otsu";
                }

                ~OtsuCalc() {
                }

                bool process(Mat& image, result_t& result) {
                    double* stddev = new double;
                    *stddev = threshold(image, discard, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
                    *stddev = 1.0;
                    result.outputs[RESULT_KEY_OTSU_STD_DEV] = stddev;
                    return true;
                }

                static Ptr<FrameProcessor> create() {
                    return makePtr<OtsuCalc>().staticCast<FrameProcessor>();
                }
        };
    }
}

#endif
