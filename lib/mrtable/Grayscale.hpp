#ifndef __GRAYSCALE_HPP__
#define __GRAYSCALE_HPP__

#include "keydefs.hpp"
#include "FrameProcessor.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace mrtable::process;
using namespace cv;

namespace mrtable {
    namespace process {
        class Grayscale : public FrameProcessor {
            public: 
                Grayscale() {
                    err = "No errors";
                    processor = "Grayscale";
                }

                ~Grayscale() {
                }

                bool process(Mat& image, result_t& result) {
                    cvtColor(image, image, CV_BGR2GRAY);
                    return true;
                }

                static Ptr<FrameProcessor> create() {
                    return makePtr<Grayscale>().staticCast<FrameProcessor>();
                }
        };
    }
}

#endif
