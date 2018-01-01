#ifndef __OUTPUTFUNCTOR_H__
#define __OUTPUTFUNCTOR_H__

#include <memory>
#include "RegionRecord.h"
#include "tbb/flow_graph.h"

using namespace std;

namespace mrtable_process {
    typedef tbb::flow::function_node< RegionRecord > OutputNodeType;

    class OutputFunctor {
        public:
            void operator()(const RegionRecord &input);
    };
}

#endif
