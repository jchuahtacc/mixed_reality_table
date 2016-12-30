#ifndef __ARUCOCOMPUTE_HPP__
#define __ARUCOCOMPUTE_HPP__

#include "Marker.hpp"
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
                vector< Marker > markers;
                int numMarkers = 100;

                ArucoCompute() {
                    ServerConfig* config = SharedData::getPtr<ServerConfig>(KEY_CONFIG);
                    server = SharedData::getPtr<TUIO::TuioServer>(KEY_TUIO_SERVER);
                    if (server == NULL) {
                        throw new std::runtime_error("ArucoCompute.hpp: No TUIO Server!");
                    }
                    
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

                    SharedData::put(RESULT_KEY_ARUCO_MARKERS, &markers);
                    SharedData::put(RESULT_KEY_ARUCO_NUM_MARKERS, &numMarkers);
                    corners = SharedData::getPtr< vector< vector< Point2f > > >(RESULT_KEY_ARUCO_CORNERS);
                    ids = SharedData::getPtr< vector< int > >(RESULT_KEY_ARUCO_IDS);
                    err = "No errors";
                    processor = "Aruco Compute";
                }

                ~ArucoCompute() {
                    SharedData::erase(RESULT_KEY_ARUCO_MARKERS);
                    SharedData::erase(RESULT_KEY_ARUCO_NUM_MARKERS);
                }

                bool process(Mat& image, result_t& result) {
                    unsigned long idx = 0;

                    for (int i = 0; i < numMarkers; i++) {
                        idx = i;
                        Marker m = markers[idx];
                        if (m.tObj != NULL) {
                            m.deathCounter++;
                        }
                        if (m.deathCounter > skippableFrames) {
                            server->removeTuioObject(m.tObj);
                            m.reset();
                        }
                    }

                    vector< int >::iterator id = ids->begin();
                    vector< vector< Point2f > >::iterator cornerVec = corners->begin();
                    result.detected += ids->size();
                    for (; id < ids->end(); id++, cornerVec++) {
                        idx = *id;
                        Marker m = markers[idx];
                        bool changed = m.calculate(*cornerVec, movementThreshold, angleThreshold);
                        if (m.tObj == NULL) {
                            // Need to update so that position is percentage of screen
                            m.tObj = server->addTuioObject(idx, m.pos.x, m.pos.y, m.rot);
                        } else if (changed) {
                            server->updateTuioObject(m.tObj, m.pos.x, m.pos.y, m.rot);
                        }

                    }
                    
                    return true;
                }

                static Ptr<FrameProcessor> create() {
                    return makePtr<ArucoCompute>().staticCast<FrameProcessor>();
                }

            private:
                vector< vector< Point2f > > *corners;
                vector< int > *ids;

                int skippableFrames = 4;
                int movementThreshold = 5;
                float angleThreshold = 0.17;
                TUIO::TuioServer* server;
        };
    }
}

#endif
