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
                
                Aruco() {
                    dictionary = ServerConfig::dictionary;
                    detectorParams = ServerConfig::detectorParameters;
                    camMatrix = ServerConfig::cameraMatrix;
                    distCoeffs = ServerConfig::distortionCoefficients;
                    markerLength = ServerConfig::markerLength;
                    err = "No errors";
                    processor = "Aruco";
                }

                ~Aruco() {
                    detectorParams.release();
                    dictionary.release();
                }

                bool process(Mat& image, Ptr< SharedData >& data, result_t& result) {
                    aruco::detectMarkers(image, dictionary, data->corners, data->ids, detectorParams, data->rejected);
                    result.detected += data->ids.size();
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
