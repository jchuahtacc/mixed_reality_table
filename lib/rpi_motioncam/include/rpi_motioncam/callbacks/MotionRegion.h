#ifndef __MOTIONREGION_H__
#define __MOTIONREGION_H__

#include <opencv2/core.hpp>
#include <memory>

using namespace std;
using namespace cv;

namespace rpi_motioncam {
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
            shared_ptr< Mat > imgPtr;
            cv::Rect roi;
    };
}


#endif /* __MOTIONREGION_H__ */
