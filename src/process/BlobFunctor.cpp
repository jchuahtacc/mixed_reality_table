#include "BlobFunctor.h"

namespace mrtable_process {
    
    cv::Ptr< cv::SimpleBlobDetector > BlobFunctor::detector = cv::SimpleBlobDetector::create();

    RegionRecord BlobFunctor::operator()(RegionRecord input) {
        if (!input.region->tag.empty() && !(input.region->tag == "blob")) {
            return input;
        }
        input.region->log_event("blob_start");
        MOTIONREGION_READ_LOCK(input.region);
        BlobFunctor::detector->detect( *(input.region->imgPtr), input.blob_keypoints);
        input.region->log_event("blob_finish");
        return input;
    }
}
