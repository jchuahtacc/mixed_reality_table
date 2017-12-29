#include "InputFunctor.h"

namespace mrtable_process {
    int InputFunctor::operator()(int input) {
        return input * 2;
    }
}
