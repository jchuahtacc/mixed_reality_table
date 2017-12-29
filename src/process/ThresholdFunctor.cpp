#include "ThresholdFunctor.h"

namespace mrtable_process {
    int ThresholdFunctor::operator()(int input) {
        return input * 2;
    }
}
