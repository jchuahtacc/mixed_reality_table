#ifndef __DISPLAYFRAME_HPP__
#define __DISPLAYFRAME_HPP__

#include "FrameProcessor.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace mrtable::data;
using namespace mrtable::process;
using namespace cv;

namespace mrtable {
    namespace process {
        class DisplayFrame : public FrameProcessor {
            public: 

                DisplayFrame(int waitTime) {
                    namedWindow("DisplayFrame", 1);
                    err = "No errors";
                    processor = "DisplayFrame";
                    this->waitTime = waitTime;
                }

                ~DisplayFrame() {
                }

                static Ptr<FrameProcessor> create(int waitTime) {
                    return makePtr<DisplayFrame>(waitTime).staticCast<FrameProcessor>();
                }

                bool process(Mat& image, Ptr< SharedData >& data, result_t& result) {
                    imshow("DisplayFrame", image);
                    data->keyPress = waitKey(waitTime);
                    return true;
                }
                int waitTime = 5;

        };
    }
}

#endif
