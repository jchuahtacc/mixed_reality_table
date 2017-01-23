#ifndef __MARKER_CPP__
#define __MARKER_CPP__

#include "Marker.h"
#include <tuio/TuioObject.h>
#include <opencv2/core/core.hpp>
#include <cmath>
#include <vector>

using namespace cv;
using namespace std;

namespace mrtable {
    namespace process {
        Marker::Marker() {
        }

        Marker::~Marker() {
        }

        void Marker::reset() {
            deathCounter = 0;
            pos.x = 0;
            pos.y = 0;
            rot = 0;
            active = false;
        }

        bool Marker::calculate(vector< Point2f >& corners, int movementThresholdPixels, float angleThreshold) {
            float newX = corners[0].x + corners[2].x / 2;
            float newY = corners[0].y + corners[2].y / 2;
            float midX = (corners[1].x + corners[2].x) / 2 - pos.x;
            float midY = (corners[1].y + corners[2].y) / 2 - pos.y;
            float newRot = atan2(midY, midX);
            if (abs(pos.x - newX) > movementThresholdPixels || abs(pos.y - newY) > movementThresholdPixels || abs(newRot - rot) > angleThreshold) { 
                float a = corners[0].x - newX;
                float b = corners[0].y - newY;
                radius = sqrt(a * a + b * b);
                pos.x = newX;
                pos.y = newY;       
                rot = newRot;
                return true;
            }
            return false;
        }
    }
}

#endif

