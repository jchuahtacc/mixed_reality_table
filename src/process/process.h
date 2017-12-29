#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <memory>
#include "tbb/flow_graph.h"
#include "InputFunctor.h"
#include "ArucoFunctor.h"
#include "OutputFunctor.h"
#include "BlobFunctor.h"
#include "ImgRecord.h"

using namespace tbb::flow;
using namespace std;

namespace mrtable_process {
    typedef function_node< shared_ptr< MotionData >, shared_ptr< ImgRecord > > InputNodeType;
    typedef function_node< shared_ptr< ImgRecord >, shared_ptr< ImgRecord > > ArucoNodeType;
    typedef function_node< shared_ptr< ImgRecord >, shared_ptr< ImgRecord > > BlobNodeType;
    typedef function_node< shared_ptr< ImgRecord > > OutputNodeType;

    class ImgProcessor {
        public:
            ImgProcessor();
            ~ImgProcessor();
            bool put(shared_ptr< MotionData > input);
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
