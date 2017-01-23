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
                    err = "No RESULT_KEY_OTSU_STD_DEV value";
                    processor = "Canny";
                }

                ~Canny() {
                }
                
                bool process(Mat& image, Ptr< SharedData >& data, result_t& result) {
                    if (data->otsu_std_dev > 0) {
                        cv::Canny(image, image, data->otsu_std_dev * 0.5, data->otsu_std_dev);
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
