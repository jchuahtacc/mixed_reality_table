#ifndef __OUTPUTFUNCTOR_H__
#define __OUTPUTFUNCTOR_H__

#include <memory>
#include <chrono>
#include "RegionRecord.h"
#include "tbb/flow_graph.h"

using namespace std;
using namespace std::chrono;

namespace mrtable_process {
    typedef tbb::flow::function_node< RegionRecord > OutputNodeType;

    class OutputFunctor {
        public:
            OutputFunctor();
            void operator()(const RegionRecord &input);
            time_point<system_clock > start;
            milliseconds aruco_duration;
            int aruco_count = 0;
    };
}

#endif
