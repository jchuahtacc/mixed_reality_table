#include "BlobFunctor.h"

namespace mrtable_process {
    int BlobFunctor::operator()(int input) {
        return input / 2;
    }
}
