#ifndef __MARKER_HPP__
#define __MARKER_HPP__

#include <tuio/TuioObject.h>
#include <opencv2/core/core.hpp>
#include <cmath>

namespace mrtable {
    namespace process {
        class Marker {
            public:
                int deathCounter = 0;
                Point2f pos;
                double rot = -1;
                TUIO::TuioObject* tObj = NULL;

                Marker() {
                }

                ~Marker() {
                }

                void reset() {
                    deathCounter = 0;
                    pos.x = 0;
                    pos.y = 0;
                    rot = 0;
                    if (tObj != NULL) {
                        delete tObj;
                        tObj = NULL;
                    }
                }

                bool calculate(vector< Point2f >& corners, int movementThreshold, float angleThreshold) {
                    float newX = corners[0].x + corners[2].x / 2;
                    float newY = corners[0].y + corners[2].y / 2;
                    float midX = (corners[1].x + corners[2].x) / 2 - pos.x;
                    float midY = (corners[1].y + corners[2].y) / 2 - pos.y;
                    float newRot = atan2(midY, midX);
                    if (abs(pos.x - newX) > movementThreshold || abs(pos.y - newY) > movementThreshold || abs(newRot - rot) > angleThreshold) { 
                        pos.x = newX;
                        pos.y = newY;       
                        rot = newRot;
                        return true;
                    }
                    return false;
                }
        };
    }
}

#endif

