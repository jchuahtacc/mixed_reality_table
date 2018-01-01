#ifndef __MOTIONVECTORCALLBACK_H__
#define __MOTIONVECTORCALLBACK_H__


#define MOTION_THRESHOLD_DEFAULT 60

#include "../typedefs.h"
#include "../RaspiCallback.h"
#include "MotionData.h"
#include "MotionFrame.h"
#include <fstream>
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
            cv::Rect calculate_roi(shared_ptr< MotionRegion > region);
            bool grow_up(shared_ptr< MotionRegion > region);
            bool grow_down(shared_ptr< MotionRegion > region);
            bool grow_left(shared_ptr< MotionRegion > region);
            bool grow_right(shared_ptr< MotionRegion > region);

            bool check_left(MMAL_BUFFER_HEADER_T *buffer, bool *searched, shared_ptr< MotionRegion > region);
            bool check_right(MMAL_BUFFER_HEADER_T *buffer, bool *searched, shared_ptr< MotionRegion > region);
            bool check_top(MMAL_BUFFER_HEADER_T *buffer, bool *searched, shared_ptr< MotionRegion > region);
            bool check_bottom(MMAL_BUFFER_HEADER_T *buffer, bool *searched, shared_ptr< MotionRegion > region);
            void grow_region(MMAL_BUFFER_HEADER_T *buffer, bool *searched, shared_ptr< MotionRegion > region);
            MotionFrame lastFrame;
        protected:
            int cols_;
            int rows_;
            int threshold_;
            int width_scale;
            int height_scale;
            RPIMOTIONCAM_OPTION_S options_;

            int buffer_count = 0;
            bool *searched;

            shared_ptr< ofstream > output;
            int frame_count = 0;

            char* lastBuffer;
        
    };
}


#endif /* __MOTIONVECTORCALLBACK_H__ */

