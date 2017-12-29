#ifndef __ARUCOCOMPUTE_HPP__
#define __ARUCOCOMPUTE_HPP__

#include "Marker.h"
#include "FrameProcessor.hpp"
#include <tuio/TuioServer.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <bitset>

using namespace mrtable::data;
using namespace mrtable::process;
using namespace cv;
using namespace cv::aruco;
using namespace std;

namespace mrtable {
    namespace process {
        class ArucoCompute : public FrameProcessor {
            public: 
                int numMarkers = 100;

                ArucoCompute() {
                    skippableFrames = ServerConfig::skippableFrames + 1;
                    movementThresholdPixels = (int)(ServerConfig::movementThreshold * ServerConfig::cameraHeight);
                    angleThreshold = ServerConfig::angleThreshold;

                    err = "No errors";
                    processor = "Aruco Compute";
                }

                ~ArucoCompute() {
                }

                bool process(Mat& image, Ptr< SharedData >& data, result_t& result) {
                    unsigned long idx = 0;

                    for (int i = 0; i < numMarkers; i++) {
                        idx = i;
                        Marker& m = data->markers[idx];
                        if (m.tObj != NULL) {
                            m.deathCounter++;
                        }
                        if (m.deathCounter > skippableFrames) {
                            data->server->removeTuioObject(m.tObj);
                            m.reset();
                        }
                    }

                    vector< int >::iterator id = data->ids.begin();
                    vector< vector< Point2f > >::iterator cornerVec = data->corners.begin();
                    result.detected += data->ids.size();
                    for (; id < data->ids.end(); id++, cornerVec++) {
                        idx = *id;
                        Marker& m = data->markers[idx];
                        bool changed = m.calculate(*cornerVec, movementThresholdPixels, angleThreshold);
                        if (m.tObj == NULL) {
                            // Need to update so that position is percentage of screen
                            Point2f pos = DetectBounds::getScreenPosition(m.pos);
                            m.tObj = data->server->addTuioObject(idx, pos.x, pos.y, m.rot);
                        } else if (changed) {
                            data->server->updateTuioObject(m.tObj, m.pos.x, m.pos.y, m.rot);
                        } 
                    }
                    
                    return true;
                }

                static Ptr<FrameProcessor> create() {
                    return makePtr<ArucoCompute>().staticCast<FrameProcessor>();
                }

            private:
                int skippableFrames = 4;
                int movementThresholdPixels = 5;
                float angleThreshold = 0.17;
                TUIO::TuioServer* server;
        };
    }
}

#endif
