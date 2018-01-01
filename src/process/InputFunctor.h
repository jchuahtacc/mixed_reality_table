#ifndef __INPUTFUNCTOR_H__
#define __INPUTFUNCTOR_H__ 

#include <memory>
#include "rpi_motioncam/RPiMotionCam.h"
#include "RegionRecord.h"
#include "tbb/flow_graph.h"

#define ARUCO_PORT 0
#define BLOB_PORT 1

using namespace tbb;
using namespace std;
using namespace rpi_motioncam;

namespace mrtable_process {
    typedef tbb::flow::multifunction_node< shared_ptr< MotionRegion >, tbb::flow::tuple< RegionRecord, RegionRecord > > InputNodeType;
    class InputFunctor {
        public:
            void operator()(const shared_ptr< MotionRegion > &region, InputNodeType::output_ports_type &ports);
    };
}

#endif
