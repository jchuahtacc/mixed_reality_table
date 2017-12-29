#ifndef __MARKER_H__
#define __MARKER_H__

#include <tuio/TuioObject.h>
#include <opencv2/core/core.hpp>
#include <cmath>
#include <vector>

using namespace cv;
using namespace std;

namespace mrtable {
    namespace process {
        class Marker {
            public:
                int deathCounter = 0;
                Point2f pos;
                float radius = 0;
                double rot = -1;
                Ptr< TUIO::TuioObject > tObj;
                bool active = false;

                Marker();

                ~Marker();
                
                void reset();

                bool calculate(vector< Point2f >& corners, int movementThresholdPixels, float angleThreshold);
        };
    }
}

#endif

