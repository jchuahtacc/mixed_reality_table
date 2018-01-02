#ifndef __MOTIONREGION_H__
#define __MOTIONREGION_H__

#define MOTIONREGION_WRITE_LOCK(region_ptr) tbb::queuing_rw_mutex::scoped_lock write_lock(*(region_ptr->imgPtr_mtx_p), true)
#define MOTIONREGION_READ_LOCK(region_ptr) tbb::queuing_rw_mutex::scoped_lock read_lock(*(region_ptr->imgPtr_mtx_p))

#include <opencv2/core.hpp>
#include <memory>
#include <chrono>
#include "tbb/queuing_rw_mutex.h"
#include "tbb/concurrent_vector.h"

using namespace std;
using namespace cv;
using namespace tbb;

namespace rpi_motioncam {
    typedef struct {
        std::chrono::time_point<std::chrono::system_clock> time;
        string event;
    } MotionRegionEvent;
    class MotionRegion {
        public:
            MotionRegion();
            MotionRegion(const MotionRegion &other);
            concurrent_vector< MotionRegionEvent > events;
            int row, col, width, height;
            bool contains(int row, int col);
           void allocate(cv::Rect roi);
            shared_ptr< Mat > imgPtr;
            cv::Rect roi;
            shared_ptr< tbb::queuing_rw_mutex > imgPtr_mtx_p; /**< Queueing RW Mutex pointer to lock imgPtr for reads/writes */
            bool mandatory;
            int id;
            string tag;
            std::chrono::time_point<std::chrono::system_clock> requested;
            std::chrono::time_point<std::chrono::system_clock> buffered;
    };
}


#endif /* __MOTIONREGION_H__ */
