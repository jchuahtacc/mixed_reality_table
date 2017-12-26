#ifndef __RAWOUTPUTCALLBACK_H__
#define __RAWOUTPUTCALLBACK_H__

#define REGION_TIMEOUT 100 

#include "RaspiCallback.h"
#include "MotionData.h"
#include <string>
#include <memory>
#include <chrono>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace rpi_motioncam {
    
    using namespace std;
    using namespace cv;
    using namespace raspivid;

    class RawOutputCallback : public RaspiCallback {
        void callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
        public:
            RawOutputCallback(int width, int height);
            void post_process();
            void copy_buffer(MMAL_BUFFER_HEADER_T *buffer);
            void save_buffer_copy(string filename);
        private:
            int width_;
            int height_;
            int size_;
            int buffer_count = 0;
            std::chrono::time_point<std::chrono::system_clock> start;
            shared_ptr< Mat > imgPtr = nullptr;
    };
}


#endif /* __RAWOUTPUTCALLBACK_H__ */

