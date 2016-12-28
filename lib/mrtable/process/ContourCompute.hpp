#ifndef __CONTOURCOMPUTE_HPP__
#define __CONTOURCOMPUTE_HPP__

#include "keydefs.hpp"
#include "Touch.hpp"
#include "FrameProcessor.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <bitset>

using namespace mrtable::process;
using namespace cv;
using namespace cv::aruco;
using namespace std;

namespace mrtable {
    namespace process {
        class ContourCompute : public FrameProcessor {
            public: 
                vector< Touches > touches;
                int numMarkers = 100;

                ContourCompute() {
                    err = "No errors";
                    processor = "Contour Compute";
                }

                ~ContourCompute() {
                    outputs->erase(RESULT_KEY_CONTOUR_TOUCHES);
                }

                void init(Ptr<ServerConfig> config) {
                    skippableFrames = config->skippableFrames + 1;
                    movementThreshold = config->movementThreshold;

                    touches.resize(50);

                    outputs->put(RESULT_KEY_CONTOUR_TOUCHES, &touches);
                }

                bool process(Mat& image, result_t& result) {
                    int dt = 3; // temporary value

                    vector< vector< Point2f > > *corners = outputs->getPtr< vector< vector< Point2f > > >(RESULT_KEY_ARUCO_CORNERS);
                    vector< int > *ids = outputs->getPtr< vector< int > >(RESULT_KEY_ARUCO_IDS);

                    unsigned long idx = 0;

                    for (int i = 0; i < numMarkers; i++) {
                        idx = i;
                        Marker m = markers[idx];
                        if (m.visible) {
                            m.deathCounter++;
                        }
                        if (m.deathCounter > skippableFrames) {
                            m.reset();
                        }
                    }

                    vector< int >::iterator id = ids->begin();
                    vector< vector< Point2f > >::iterator cornerVec = corners->begin();
                    for (; id < ids->end(); id++, cornerVec++) {
                        idx = *id;
                        Marker m = markers[idx];
                        m.visible = true;
                        m.calculate(*cornerVec, movementThreshold, angleThreshold, dt);
                    }
                    
                    return true;
                }

                static Ptr<FrameProcessor> create() {
                    return makePtr<ContourCompute>().staticCast<FrameProcessor>();
                }

            private:
                int skippableFrames = 4;
                int movementThreshold = 5;
                float angleThreshold = 0.17;
        };
    }
}

#endif
