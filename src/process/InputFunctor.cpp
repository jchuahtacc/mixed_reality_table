#include "InputFunctor.h"

namespace mrtable_process {
    shared_ptr< ImgRecord > InputFunctor::operator()(shared_ptr< MotionData > input) {
        return std::make_shared< ImgRecord >(input);
    }
}
