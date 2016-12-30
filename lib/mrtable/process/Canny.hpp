#ifndef __CANNY_HPP__
#define __CANNY_HPP__

#include "FrameProcessor.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace mrtable::data;
using namespace mrtable::process;
using namespace cv;

namespace mrtable {
    namespace process {
        class Canny : public FrameProcessor {
            public: 
                Canny() {
                    if (SharedData::has(RESULT_KEY_OTSU_STD_DEV)) {
                        stddev = SharedData::get<double>(RESULT_KEY_OTSU_STD_DEV);
                    }
                    err = "No RESULT_KEY_OTSU_STD_DEV value";
                    processor = "Canny";
                }

                ~Canny() {
                }
                
                void init(Ptr<ServerConfig> config) {
                }

                bool process(Mat& image, result_t& result) {
                    if (stddev > 0) {
                        cv::Canny(image, image, stddev * 0.5, stddev);
                        return true;
                    }
                    cv::Canny(image, image, 100, 300);
                    return false;
                }

                static Ptr<FrameProcessor> create() {
                    return makePtr<Canny>().staticCast<FrameProcessor>();
                }

                double stddev = -1;

        };
    }
}

#endif
