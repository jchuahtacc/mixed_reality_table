#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <memory>
#include "tbb/flow_graph.h"
#include "InputFunctor.h"
#include "ArucoFunctor.h"
#include "OutputFunctor.h"
#include "BlobFunctor.h"
#include "RegionRecord.h"

using namespace tbb::flow;
using namespace std;

namespace mrtable_process {
    class ImgProcessor {
        public:
            ImgProcessor();
            ~ImgProcessor();
            bool put(shared_ptr< MotionRegion > input);
        private:
            graph g;
            shared_ptr< InputNodeType > input_node;
            shared_ptr< OutputNodeType > output_node;
            shared_ptr< ArucoNodeType > aruco_node;
            shared_ptr< BlobNodeType > blob_node;
            shared_ptr< InputFunctor > input_functor;
            shared_ptr< OutputFunctor > output_functor;
            shared_ptr< ArucoFunctor > aruco_functor;
            shared_ptr< BlobFunctor > blob_functor;
    };
};

#endif
