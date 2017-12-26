#ifndef __MOTIONVECTORCALLBACK_H__
#define __MOTIONVECTORCALLBACK_H__


#define MOTION_THRESHOLD 60

#include "raspivid/RaspiCallback.h"
#include "rpi_motioncam/MotionData.h"
#include <memory>
#include <vector>

namespace rpi_motioncam {

    using namespace std;
    using namespace raspivid;

    class MotionVectorCallback : public RaspiCallback {
        public: 
            MotionVectorCallback(int width, int height);
            ~MotionVectorCallback();
            void callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
            void post_process();
            int buffer_pos(int row, int col);
            bool check_left(MMAL_BUFFER_HEADER_T *buffer, bool *searched, MotionRegion &region);
            bool check_right(MMAL_BUFFER_HEADER_T *buffer, bool *searched, MotionRegion &region);
            bool check_top(MMAL_BUFFER_HEADER_T *buffer, bool *searched, MotionRegion &region);
            bool check_bottom(MMAL_BUFFER_HEADER_T *buffer, bool *searched, MotionRegion &region);
            void grow_region(MMAL_BUFFER_HEADER_T *buffer, bool *searched, MotionRegion &region);
        protected:
            int cols_;
            int rows_;

            // vector< MotionRegion> regions;
            shared_ptr< vector< MotionRegion > > lastRegions = nullptr;
            int buffer_count = 0;
            bool *searched;
        
    };
}


#endif /* __MOTIONVECTORCALLBACK_H__ */

