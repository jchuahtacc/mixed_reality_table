#ifndef __TOUCH_CPP__
#define __TOUCH_CPP__

#include "Touch.h"
#include <tuio/TuioCursor.h>
#include <opencv2/core/core.hpp>
#include <cmath>

namespace mrtable {
    namespace process {
        Touch::Touch() {
        }

        Touch::~Touch() {
        }

        void Touch::reset() {
            deathCounter = 0;
            bounds.x = 0;
            bounds.y = 0;
            bounds.width = 0;
            bounds.height = 0;
            pos.x = 0;
            pos.y = 0;
            active = false;
        }

        bool Touch::calculate(Rect newRect, int movementThreshold) {
            // TODO: Need to reconcile for screen size/rate
            float newX = newRect.x + newRect.width / 2;
            float newY = newRect.y + newRect.width / 2;
            if (abs(pos.x - newX) > movementThreshold || abs(pos.y - newY) > movementThreshold) { 
                pos.x = newX;
                pos.y = newY;       
                bounds = newRect;
                return true;
            }
            return false;
        }
    }
}

#endif

