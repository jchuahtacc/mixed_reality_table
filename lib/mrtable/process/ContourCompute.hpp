#ifndef __CONTOURCOMPUTE_HPP__
#define __CONTOURCOMPUTE_HPP__

#include "Touch.hpp"
#include "FrameProcessor.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <bitset>
#include <iostream>

using namespace mrtable::data;
using namespace mrtable::process;
using namespace cv;
using namespace cv::aruco;
using namespace std;

namespace mrtable {
    namespace process {
        class ContourCompute : public FrameProcessor {
            public: 
                vector< Touch > touches;
                Ptr< vector< Message > > msgVector;
                int numMarkers = 100;

                ContourCompute() {
                    ServerConfig* config = SharedData::getPtr<ServerConfig>(KEY_CONFIG);
                    msgVector = makePtr< vector< Message > >();
                    MessageBroker::bind(CMD_PUT_REGION, msgVector);
                    skippableFrames = config->skippableFrames + 1;
                    movementThreshold = config->movementThreshold;
                    params = config->contourParameters;

                    // temporary values
                    minPoints = params->minPointRatio * 640;
                    maxPoints = params->maxPointRatio * 480;
                    minWidth = params->minWidthRatio * 640;
                    maxWidth = params->maxWidthRatio * 480;

                    touches.resize(50);
                    contours = SharedData::getPtr< vector< vector<Point> > >(RESULT_KEY_CONTOUR_CONTOURS);
                    hierarchy = SharedData::getPtr< vector<Vec4i> >(RESULT_KEY_CONTOUR_HIERARCHY);

                    SharedData::put(RESULT_KEY_CONTOUR_TOUCHES, &touches);
                    err = "No errors";
                    processor = "Contour Compute";
                }

                ~ContourCompute() {
                    SharedData::erase(RESULT_KEY_CONTOUR_TOUCHES);
                    msgVector.release();
                    params.release();
                }

                bool process(Mat& image, result_t& result) {
                    for ( vector< Message >::iterator msg = msgVector->begin(); msg < msgVector->end(); msg++) {
                        std::cout << "ContourCompute received cmdCode " << msg->cmdCode << " with params " << msg->params << " at " << msg->time.getSeconds() << std::endl; 
                    }
                    msgVector->clear();
                    /*
                    vector< vector<Point> >::iterator contour = contours->begin();
                    for (; contour  < contours->end(); contour++) {
                        if (contour->size() > minPoints && contour->size() < maxPoints) {
                            Rect r = cv::boundingRect(*contour);
                        }
                    }
                    */
                    return true;
                }

                static Ptr<FrameProcessor> create() {
                    return makePtr<ContourCompute>().staticCast<FrameProcessor>();
                }

            private:
                int skippableFrames = 4;
                int movementThreshold = 5;
                int minPoints = 100;
                int maxPoints = 300;
                int minWidth = 50;
                int maxWidth = 100;
                vector< vector<Point> > *contours;
                vector<Vec4i> *hierarchy;
                Ptr< mrtable::config::ContourParams > params;
        };
    }
}

#endif
