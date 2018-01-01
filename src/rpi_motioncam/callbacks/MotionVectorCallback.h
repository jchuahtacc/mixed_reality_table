#ifndef __MOTIONVECTORCALLBACK_H__
#define __MOTIONVECTORCALLBACK_H__


#define MOTION_THRESHOLD_DEFAULT 60

#include "../typedefs.h"
#include "../RaspiCallback.h"
#include "MotionData.h"
#include "MotionFrame.h"
#include "VectorPreview.h"
#include "FramePreview.h"
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

            bool check_left(shared_ptr< MotionRegion > region);
            bool check_right(shared_ptr< MotionRegion > region);
            bool check_top(shared_ptr< MotionRegion > region);
            bool check_bottom(shared_ptr< MotionRegion > region);
            void grow_region(shared_ptr< MotionRegion > region);
            MotionFrame lastFrame;
        protected:
            int cols_;
            int rows_;
            int width_scale;
            int height_scale;
            int elements_;
            shared_ptr< VectorPreview > vector_preview;
            shared_ptr< FramePreview > frame_preview;
            RPIMOTIONCAM_OPTION_S options_;

            int buffer_count = 0;
            char* searched;

            int frame_count = 0;

            char* lastBuffer;
            bool got_vectors = false;
        
    };
}


#endif /* __MOTIONVECTORCALLBACK_H__ */

