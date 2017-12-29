#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <memory>
#include "tbb/flow_graph.h"
#include "ThresholdFunctor.h"
#include "OutputFunctor.h"

using namespace tbb::flow;
using namespace std;

namespace mrtable_process {
    typedef function_node< int, int > ThresholdNodeType;
    typedef function_node< int > OutputNodeType;

    class ImgProcessor {
        public:
            ImgProcessor();
            ~ImgProcessor();
            bool put(int input);
        private:
            graph g;
            shared_ptr< ThresholdNodeType > threshold_node;
            shared_ptr< OutputNodeType > output_node;
            shared_ptr< ThresholdFunctor > threshold_functor;
            shared_ptr< OutputFunctor > output_functor;
    };
};

#endif
