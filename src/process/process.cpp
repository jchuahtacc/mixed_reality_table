#include "process.h"

namespace mrtable_process {
    ImgProcessor::ImgProcessor() {
        aruco_functor = shared_ptr< ArucoFunctor >( new ArucoFunctor() );
        blob_functor = shared_ptr< BlobFunctor >( new BlobFunctor() );
        output_functor = shared_ptr< OutputFunctor >( new OutputFunctor() );
        aruco_node = shared_ptr< ArucoNodeType >(new ArucoNodeType(g, tbb::flow::unlimited, *aruco_functor));
        blob_node = shared_ptr< BlobNodeType >(new BlobNodeType(g, tbb::flow::unlimited, *blob_functor));
        output_node = shared_ptr< OutputNodeType >(new OutputNodeType(g, 1, *output_functor));
        make_edge( *aruco_node, *blob_node);
        make_edge( *blob_node, *output_node);
    }

    ImgProcessor::~ImgProcessor() {
        g.wait_for_all();
    }

    bool ImgProcessor::put(shared_ptr< MotionRegion > region) {
        //return aruco_node->try_put(RegionRecord(region, "aruco"));
        return aruco_node->try_put(RegionRecord(region, "aruco"));
    }
}
