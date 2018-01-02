#include "BlobFunctor.h"

namespace mrtable_process {
    
    cv::Ptr< cv::SimpleBlobDetector > BlobFunctor::detector = cv::SimpleBlobDetector::create();

    RegionRecord BlobFunctor::operator()(RegionRecord input) {
        if (!input.region->tag.empty() && !(input.region->tag == "blob")) {
            return input;
        }
        //cout << "Blob " << input.region->id << endl;
        MOTIONREGION_READ_LOCK(input.region);
        BlobFunctor::detector->detect( *(input.region->imgPtr), input.blob_keypoints);
        return input;
    }
}
