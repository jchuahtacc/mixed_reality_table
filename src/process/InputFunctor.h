#ifndef __INPUTFUNCTOR_H__
#define __INPUTFUNCTOR_H__ 

#include <memory>
#include "rpi_motioncam/RPiMotionCam.h"
#include "ImgRecord.h"

using namespace std;
using namespace rpi_motioncam;

namespace mrtable_process {
    class InputFunctor {
        public:
            shared_ptr< ImgRecord >  operator()(shared_ptr< MotionData > input);
    };
}

#endif
