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

                int keypress = 0;

                DisplayFrame(int waitTime) {
                    SharedData::put(RESULT_KEY_DISPLAYFRAME_KEYPRESS, &keypress);
                    namedWindow("DisplayFrame", 1);
                    err = "No errors";
                    processor = "DisplayFrame";
                    this->waitTime = waitTime;
                }

                ~DisplayFrame() {
                    SharedData::erase(RESULT_KEY_DISPLAYFRAME_KEYPRESS);
                }

                static Ptr<FrameProcessor> create(int waitTime) {
                    return makePtr<DisplayFrame>(waitTime).staticCast<FrameProcessor>();
                }

                bool process(Mat& image, result_t& result) {
                    imshow("DisplayFrame", image);
                    keypress = waitKey(waitTime);
                    return true;
                }
                int waitTime = 5;

        };
    }
}

#endif
