#include "BlobFunctor.h"

namespace mrtable_process {
    shared_ptr< ImgRecord > BlobFunctor::operator()(shared_ptr< ImgRecord > input) {
        return input;
    }
}
