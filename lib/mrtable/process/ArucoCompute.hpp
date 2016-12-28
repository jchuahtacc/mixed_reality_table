#ifndef __ARUCOCOMPUTE_HPP__
#define __ARUCOCOMPUTE_HPP__

#include "keydefs.hpp"
#include "Marker.hpp"
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
        class ArucoCompute : public FrameProcessor {
            public: 
                vector< Marker > markers;
                int numMarkers = 100;

                ArucoCompute() {
                    err = "No errors";
                    processor = "Aruco Compute";
                }

                ~ArucoCompute() {
                    outputs->erase(RESULT_KEY_ARUCO_MARKERS);
                    outputs->erase(RESULT_KEY_ARUCO_NUM_MARKERS);
                }

                void init(Ptr<ServerConfig> config) {
                    skippableFrames = config->skippableFrames + 1;
                    movementThreshold = config->movementThreshold;
                    angleThreshold = config->angleThreshold;

                    if (config->dictionaryId == cv::aruco::DICT_ARUCO_ORIGINAL) {
                        numMarkers = 1024;
                    } else {
                        switch (config->dictionaryId % 4) {
                            case 0: numMarkers = 50; break;
                            case 1: numMarkers = 100; break;
                            case 2: numMarkers = 250; break;
                            case 4: numMarkers = 1000; break;
                        }
                    }

                    markers.resize(numMarkers);

                    outputs->put(RESULT_KEY_ARUCO_MARKERS, &markers);
                    outputs->put(RESULT_KEY_ARUCO_NUM_MARKERS, &numMarkers);
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
                    return makePtr<ArucoCompute>().staticCast<FrameProcessor>();
                }

            private:
                int skippableFrames = 4;
                int movementThreshold = 5;
                float angleThreshold = 0.17;
        };
    }
}

#endif
