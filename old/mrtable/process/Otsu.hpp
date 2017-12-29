#ifndef __OTSU_HPP__
#define __OTSU_HPP__

#include "FrameProcessor.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace mrtable::data;
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


                bool process(Mat& image, Ptr< SharedData >& data, result_t& result) {
                    data->otsu_std_dev = threshold(image, image, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
                    return true;
                }

                static Ptr<FrameProcessor> create() {
                    return makePtr<Otsu>().staticCast<FrameProcessor>();
                }
        };
    }
}

#endif
