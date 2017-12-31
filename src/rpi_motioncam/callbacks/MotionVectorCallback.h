#ifndef __MOTIONVECTORCALLBACK_H__
#define __MOTIONVECTORCALLBACK_H__


#define MOTION_THRESHOLD_DEFAULT 60

#include "../typedefs.h"
#include "../RaspiCallback.h"
#include "MotionData.h"
#include <memory>
#include <vector>

using namespace std;
using namespace rpi_motioncam;

namespace rpi_motioncam {

    class MotionVectorCallback : public RaspiCallback {
        public: 
            MotionVectorCallback(RPIMOTIONCAM_OPTION_S options);
            ~MotionVectorCallback();
            void callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
            void post_process();
            int buffer_pos(int row, int col);
            cv::Rect calculate_roi(const MotionRegion &region);
            bool grow_up(MotionRegion &region);
            bool grow_down(MotionRegion &region);
            bool grow_left(MotionRegion &region);
            bool grow_right(MotionRegion &region);

            bool check_left(MMAL_BUFFER_HEADER_T *buffer, bool *searched, MotionRegion &region);
            bool check_right(MMAL_BUFFER_HEADER_T *buffer, bool *searched, MotionRegion &region);
            bool check_top(MMAL_BUFFER_HEADER_T *buffer, bool *searched, MotionRegion &region);
            bool check_bottom(MMAL_BUFFER_HEADER_T *buffer, bool *searched, MotionRegion &region);
            void grow_region(MMAL_BUFFER_HEADER_T *buffer, bool *searched, MotionRegion &region);
        protected:
            int cols_;
            int rows_;
            int threshold_;
            int width_scale;
            int height_scale;
            RPIMOTIONCAM_OPTION_S options_;

            // vector< MotionRegion> regions;
            shared_ptr< vector< MotionRegion > > lastRegions = nullptr;
            int buffer_count = 0;
            bool *searched;
        
    };
}


#endif /* __MOTIONVECTORCALLBACK_H__ */

