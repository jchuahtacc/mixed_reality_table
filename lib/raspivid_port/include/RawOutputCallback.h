#ifndef __RAWOUTPUTCALLBACK_H__
#define __RAWOUTPUTCALLBACK_H__

#include "RaspiCallback.h"
#include <memory>
#include <chrono>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

namespace raspivid {
    class RawOutputCallback : public RaspiCallback {
        void callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
        public:
            RawOutputCallback();
            void post_process();
        private:
            bool written = false;
            int frame_num = 0;
            int buffer_count = 0;
            std::chrono::time_point<std::chrono::system_clock> start;
            shared_ptr< Mat > imgPtr;
    };
}


#endif /* __RAWOUTPUTCALLBACK_H__ */

