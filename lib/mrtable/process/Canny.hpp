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
                
                void init(Ptr<ServerConfig> config) {
                }

                bool process(Mat& image, result_t& result) {
                    if (outputs->has(RESULT_KEY_OTSU_STD_DEV)) {
                        double stddev = outputs->get<double>(RESULT_KEY_OTSU_STD_DEV);
                        cv::Canny(image, image, stddev * 0.5, stddev);
                        return true;
                    }
                    cv::Canny(image, image, 100, 300);
                    return false;
                }

                static Ptr<FrameProcessor> create() {
                    return makePtr<Canny>().staticCast<FrameProcessor>();
                }

        };
    }
}

#endif
