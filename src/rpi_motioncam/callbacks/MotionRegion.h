#ifndef __MOTIONREGION_H__
#define __MOTIONREGION_H__

#include <opencv2/core.hpp>
#include <memory>
#include "tbb/queuing_rw_mutex.h"

using namespace std;
using namespace cv;

namespace rpi_motioncam {
    class MotionRegion {
        public:
            MotionRegion();
            MotionRegion(const MotionRegion &other);
            int row, col, width, height;
            bool contains(int row, int col);
            bool grow_up();
            bool grow_down();
            bool grow_left();
            bool grow_right();
            void allocate(cv::Rect roi);
            shared_ptr< Mat > imgPtr;
            cv::Rect roi;
            shared_ptr< tbb::queuing_rw_mutex > imgPtr_mtx_p; /**< Queueing RW Mutex pointer to lock imgPtr for reads/writes */
    };
}


#endif /* __MOTIONREGION_H__ */
