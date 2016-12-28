#ifndef __ARUCO_HPP__
#define __ARUCO_HPP__

#include "keydefs.hpp"
#include "FrameProcessor.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace mrtable::process;
using namespace cv;
using namespace cv::aruco;

namespace mrtable {
    namespace process {
        class Aruco : public FrameProcessor {
            public: 
                vector< int > ids;
                vector< vector< Point2f > > corners, rejected;
                
                Aruco() {
                    err = "No errors";
                    processor = "Aruco";
                }

                ~Aruco() {
                    outputs->erase(RESULT_KEY_ARUCO_IDS);
                    outputs->erase(RESULT_KEY_ARUCO_CORNERS);
                    outputs->erase(RESULT_KEY_ARUCO_REJECTED);
                    detectorParams.release();
                    dictionary.release();
                }

                void init(Ptr<ServerConfig> config) {
                    dictionary = config->dictionary;
                    detectorParams = config->detectorParameters;
                    camMatrix = config->cameraMatrix;
                    distCoeffs = config->distortionCoefficients;
                    markerLength = config->markerLength;
                    outputs->put(RESULT_KEY_ARUCO_IDS, &ids);
                    outputs->put(RESULT_KEY_ARUCO_CORNERS, &corners);
                    outputs->put(RESULT_KEY_ARUCO_REJECTED, &rejected);
                }

                bool process(Mat& image, result_t& result) {
                    aruco::detectMarkers(image, dictionary, corners, ids, detectorParams, rejected);
                    result.detected += ids.size();
                    return true;
                }

                static Ptr<FrameProcessor> create() {
                    return makePtr<Otsu>().staticCast<FrameProcessor>();
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
