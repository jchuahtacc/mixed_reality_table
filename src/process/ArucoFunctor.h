#ifndef __ARUCOFUNCTOR_H__
#define __ARUCOFUNCTOR_H__

#include <opencv2/aruco.hpp>

using namespace cv::aruco;

namespace mrtable_process {
    class ArucoFunctor {
        public:
            int operator()(int input);
    };
}

#endif
