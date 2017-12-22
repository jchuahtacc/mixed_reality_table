#ifndef __MOTIONVECTORCALLBACK_H__
#define __MOTIONVECTORCALLBACK_H__


#define MOTION_THRESHOLD 60

#include "RaspiCallback.h"
#include <memory>
#include <vector>

using namespace std;

namespace raspivid {

    class MotionRegion {
        public:
            MotionRegion(int row, int col);
            int row, col, width, height;
            bool contains(int row, int col);
            bool grow_up();
            bool grow_down();
            bool grow_left();
            bool grow_right();
            static int num_rows;
            static int num_cols;

    };

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
        // protected:
            int cols_;
            int rows_;

            bool new_vectors;
            vector< MotionRegion> regions;
        private:
            bool *searched;
        
    };
}


#endif /* __MOTIONVECTORCALLBACK_H__ */

