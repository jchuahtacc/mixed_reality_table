#ifndef __OTSU_HPP__
#define __OTSU_HPP__

#include "keydefs.hpp"
#include "FrameProcessor.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace mrtable::process;
using namespace cv;

namespace mrtable {
    namespace process {
        class Otsu : public FrameProcessor {
            public: 
                double stddev = 200;

                Otsu() {
                    err = "No errors";
                    processor = "Otsu";
                }

                ~Otsu() {
                }

                void init(Ptr<ServerConfig> config) {
                    outputs->put(RESULT_KEY_OTSU_STD_DEV, &stddev);
                }

                bool process(Mat& image, result_t& result) {
                    stddev = threshold(image, image, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
                    return true;
                }

                static Ptr<FrameProcessor> create() {
                    return makePtr<Otsu>().staticCast<FrameProcessor>();
                }
        };
    }
}

#endif
