#ifndef __REGIONCALLBACK_H__
#define __REGIONCALLBACK_H__

#include "callbacks/MotionRegion.h"
#include <memory>

using namespace std;

namespace rpi_motioncam {
    class RegionCallback {
        public:
            virtual void process(shared_ptr< MotionRegion > region) = 0;
    };
}

#endif
