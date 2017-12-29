#ifndef __OTSUCALC_HPP__
#define __OTSUCALC_HPP__

#include "FrameProcessor.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace mrtable::data;
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

                bool process(Mat& image, Ptr< SharedData >& data, result_t& result) {
                    data->otsu_std_dev = threshold(image, discard, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
                    return true;
                }

                static Ptr<FrameProcessor> create() {
                    return makePtr<OtsuCalc>().staticCast<FrameProcessor>();
                }
        };
    }
}

#endif
