#include "ArucoFunctor.h"

using namespace std;

namespace mrtable_process {
    shared_ptr< ImgRecord > ArucoFunctor::operator()(shared_ptr< ImgRecord > input) {
        return input;
    }
}
