#ifndef __BLOBFUNCTOR_H__
#define __BLOBFUNCTOR_H__

#include <opencv2/features2d.hpp>
#include <memory>
#include "ImgRecord.h"

using namespace cv;
using namespace std;

namespace mrtable_process {
    class BlobFunctor {
        public:
            shared_ptr< ImgRecord > operator()(shared_ptr< ImgRecord > input);
    };
}

#endif
