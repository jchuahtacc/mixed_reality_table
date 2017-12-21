#ifndef __MOTIONVECTORCALLBACK_H__
#define __MOTIONVECTORCALLBACK_H__

#include "RaspiCallback.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <memory>

using namespace std;
using namespace cv;

namespace raspivid {
    class MotionVectorCallback : public RaspiCallback {
        public: 
            MotionVectorCallback(int width, int height);
            void callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
            void post_process();
        protected:
            int cols_;
            int rows_;

            bool fresh;
            Mat motion;
        
    };
}


#endif /* __MOTIONVECTORCALLBACK_H__ */

