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
            static int last_row;
            static int last_col;

    };

    class MotionVectorCallback : public RaspiCallback {
        public: 
            MotionVectorCallback(int width, int height);
            void callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
            void post_process();
            int buffer_pos(int row, int col);
            bool check_col(MMAL_BUFFER_HEADER_T *buffer, bool *searched, int row, int col, int height);
            bool check_row(MMAL_BUFFER_HEADER_T *buffer, bool *searched, int row, int col, int width);
        protected:
            int cols_;
            int rows_;

            bool new_vectors;
            vector< MotionRegion> regions;
        
    };
}


#endif /* __MOTIONVECTORCALLBACK_H__ */

