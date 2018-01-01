#include "process.h"

namespace mrtable_process {
    ImgProcessor::ImgProcessor() {
        input_functor = shared_ptr< InputFunctor >( new InputFunctor() );
        aruco_functor = shared_ptr< ArucoFunctor >( new ArucoFunctor() );
        blob_functor = shared_ptr< BlobFunctor >( new BlobFunctor() );
        output_functor = shared_ptr< OutputFunctor >( new OutputFunctor() );
        input_node = shared_ptr< InputNodeType >(new InputNodeType(g, 1, *input_functor));
        aruco_node = shared_ptr< ArucoNodeType >(new ArucoNodeType(g, tbb::flow::unlimited, *aruco_functor));
        blob_node = shared_ptr< BlobNodeType >(new BlobNodeType(g, tbb::flow::unlimited, *blob_functor));
        output_node = shared_ptr< OutputNodeType >(new OutputNodeType(g, 1, *output_functor));

        output_port< ARUCO_PORT >( *input_node ).register_successor( *aruco_node );
        output_port< BLOB_PORT >( *input_node ).register_successor( *blob_node );

        //make_edge( *input_node, *aruco_node);
        //make_edge( *input_node, *blob_node);
        make_edge( *aruco_node, *output_node);
        make_edge( *blob_node, *output_node);
    }

    ImgProcessor::~ImgProcessor() {
        g.wait_for_all();
    }

    bool ImgProcessor::put(shared_ptr< MotionRegion > input) {
        return input_node->try_put(input);
    }
}
