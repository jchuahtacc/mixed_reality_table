#ifndef __TOUCH_HPP__
#define __TOUCH_HPP__

#include <tuio/TuioCursor.h>
#include <opencv2/core/core.hpp>
#include <cmath>

namespace mrtable {
    namespace process {
        class Touch {
            public:
                Point2f pos;
                Rect bounds;
                int deathCounter = 0;
                TUIO::TuioCursor *tCur;

                Touch() {
                }

                ~Touch() {
                }

                void reset() {
                    deathCounter = 0;
                    bounds.x = 0;
                    bounds.y = 0;
                    bounds.width = 0;
                    bounds.height = 0;
                    pos.x = 0;
                    pos.y = 0;
                    if (tCur != NULL) {
                        delete tCur;
                        tCur = NULL;
                    }
                }

                bool calculate(Rect newRect, int movementThreshold) {
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
        };
    }
}

#endif

