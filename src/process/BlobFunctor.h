#ifndef __BLOBFUNCTOR_H__
#define __BLOBFUNCTOR_H__

#include <opencv2/features2d.hpp>

using namespace cv;

namespace mrtable_process {
    class BlobFunctor {
        public:
            int operator()(int input);
    };
}

#endif
