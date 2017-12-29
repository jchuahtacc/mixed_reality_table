#ifndef __ARUCOFUNCTOR_H__
#define __ARUCOFUNCTOR_H__

#include <opencv2/aruco.hpp>
#include <memory>
#include "ImgRecord.h"

using namespace cv::aruco;
using namespace std;

namespace mrtable_process {
    class ArucoFunctor {
        public:
            shared_ptr< ImgRecord >  operator()(shared_ptr< ImgRecord > input);
    };
}

#endif
