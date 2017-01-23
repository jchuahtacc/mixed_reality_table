#ifndef __TOUCH_H__
#define __TOUCH_H__

#include <tuio/TuioCursor.h>
#include <opencv2/core/core.hpp>
#include <cmath>

using namespace cv;

namespace mrtable {
    namespace process {
        class Touch {
            public:
                Point2f pos;
                Rect bounds;
                int deathCounter = 0;
                bool active = false;
                Ptr< TUIO::TuioCursor > tCur;

                Touch();

                ~Touch();

                void reset();

                bool calculate(Rect newRect, int movementThreshold);
        };
    }
}

#endif

