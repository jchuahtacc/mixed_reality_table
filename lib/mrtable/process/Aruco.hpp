#ifndef __ARUCO_HPP__
#define __ARUCO_HPP__

#include "FrameProcessor.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace mrtable::process;
using namespace cv;
using namespace cv::aruco;
using namespace mrtable::data;

namespace mrtable {
    namespace process {
        class Aruco : public FrameProcessor {
            public: 
                vector< int > ids;
                vector< vector< Point2f > > corners, rejected;
                
                Aruco() {
                    ServerConfig* config = SharedData::getPtr<ServerConfig>(KEY_CONFIG);
                    dictionary = config->dictionary;
                    detectorParams = config->detectorParameters;
                    camMatrix = config->cameraMatrix;
                    distCoeffs = config->distortionCoefficients;
                    markerLength = config->markerLength;
                    SharedData::put(RESULT_KEY_ARUCO_IDS, &ids);
                    SharedData::put(RESULT_KEY_ARUCO_CORNERS, &corners);
                    SharedData::put(RESULT_KEY_ARUCO_REJECTED, &rejected);

                    err = "No errors";
                    processor = "Aruco";
                }

                ~Aruco() {
                    SharedData::erase(RESULT_KEY_ARUCO_IDS);
                    SharedData::erase(RESULT_KEY_ARUCO_CORNERS);
                    SharedData::erase(RESULT_KEY_ARUCO_REJECTED);
                    detectorParams.release();
                    dictionary.release();
                }

                bool process(Mat& image, result_t& result) {
                    aruco::detectMarkers(image, dictionary, corners, ids, detectorParams, rejected);
                    result.detected += ids.size();
                    return true;
                }

                static Ptr<FrameProcessor> create() {
                    return makePtr<Aruco>().staticCast<FrameProcessor>();
                }

            private:
                Ptr<aruco::DetectorParameters> detectorParams;
                Ptr<aruco::Dictionary> dictionary;
                Mat camMatrix, distCoeffs;
                double markerLength = 0.1;
        };
    }
}

#endif
