#ifndef __BLOBFUNCTOR_H__
#define __BLOBFUNCTOR_H__

#include <opencv2/features2d.hpp>
#include <memory>
#include "RegionRecord.h"
#include "tbb/flow_graph.h"

using namespace cv;
using namespace std;

namespace mrtable_process {
    typedef tbb::flow::function_node< RegionRecord, RegionRecord > BlobNodeType;

    class BlobFunctor {
        public:
            RegionRecord operator()(RegionRecord input);
            static cv::Ptr< SimpleBlobDetector > detector;
    };
}

#endif
