#include "process.h"

namespace mrtable_process {
    ImgProcessor::ImgProcessor() {
        threshold_functor = shared_ptr< ThresholdFunctor >(new ThresholdFunctor());
        output_functor = shared_ptr< OutputFunctor >(new OutputFunctor());
        threshold_node = shared_ptr< ThresholdNodeType >(new ThresholdNodeType(g, 1, *threshold_functor));
        output_node = shared_ptr< OutputNodeType >(new OutputNodeType(g, 1, *output_functor));
        make_edge( *threshold_node, *output_node);
    }

    ImgProcessor::~ImgProcessor() {
        g.wait_for_all();
    }

    bool ImgProcessor::put(int input) {
        return threshold_node->try_put(input);
    }
}
