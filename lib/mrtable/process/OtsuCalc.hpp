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
                double stddev = 200;

                OtsuCalc() {
                    err = "No errors";
                    processor = "Otsu";
                }

                ~OtsuCalc() {
                    outputs->erase(RESULT_KEY_OTSU_STD_DEV);
                }

                void init(Ptr<ServerConfig> config) {
                    outputs->put(RESULT_KEY_OTSU_STD_DEV, &stddev);
                }

                bool process(Mat& image, result_t& result) {
                    stddev = threshold(image, discard, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
                    return true;
                }

                static Ptr<FrameProcessor> create() {
                    return makePtr<OtsuCalc>().staticCast<FrameProcessor>();
                }
        };
    }
}

#endif
