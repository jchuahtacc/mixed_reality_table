#ifndef __ARUCOFUNCTOR_H__
#define __ARUCOFUNCTOR_H__

#include <opencv2/aruco.hpp>
#include <memory>
#include "RegionRecord.h"
#include "tbb/flow_graph.h"

using namespace cv::aruco;
using namespace std;

namespace mrtable_process {
    typedef tbb::flow::function_node< RegionRecord, RegionRecord > ArucoNodeType;
    class ArucoFunctor {
        public:
            RegionRecord operator()( RegionRecord input );
    };
}

#endif
