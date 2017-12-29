#include "ArucoFunctor.h"

namespace mrtable_process {
    int ArucoFunctor::operator()(int input) {
        return input * input;
    }
}
