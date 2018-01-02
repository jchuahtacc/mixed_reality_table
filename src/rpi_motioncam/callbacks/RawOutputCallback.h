#ifndef __RAWOUTPUTCALLBACK_H__
#define __RAWOUTPUTCALLBACK_H__

#define REGION_TIMEOUT 100 

#include "rpi_motioncam/RaspiCallback.h"
#include "rpi_motioncam/callbacks/MotionData.h"
#include "rpi_motioncam/RegionCallback.h"
#include <string>
#include <memory>
#include <chrono>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

namespace rpi_motioncam {
    class RawOutputCallback : public RaspiCallback {
        void callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
        public:
            RawOutputCallback(int width, int height, std::shared_ptr< RegionCallback > callback);
            void post_process();
            void copy_buffer(MMAL_BUFFER_HEADER_T *buffer);
            void save_buffer_copy(string filename);
        private:
            int width_;
            int height_;
            int size_;
            int buffer_count = 0;
            std::chrono::time_point<std::chrono::system_clock> start;
            std::shared_ptr< Mat > imgPtr = nullptr;
            std::shared_ptr< RegionCallback > region_callback = nullptr;
    };
}


#endif /* __RAWOUTPUTCALLBACK_H__ */

