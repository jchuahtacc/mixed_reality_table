
#ifndef __PARSERS_H__
#define __PARSERS_H__

#include <iostream>
#include <vector>
#include <opencv2/aruco.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d.hpp>
#include <fstream>
#include <streambuf>
#include <string>
#include <sstream>

using namespace std;
using namespace cv;
using namespace cv::aruco;

namespace mrtable {
    namespace config {
        bool readCameraParameters(string filename, Mat &camMatrix, Mat &distCoeffs);
        bool writeCameraParameters(string filename, Mat camMatrix, Mat distCoeffs);
        bool readBlobParameters(string filename, SimpleBlobDetector::Params *params);
        bool writeBlobParameters(string filename, SimpleBlobDetector::Params *params);
        bool readDetectorParameters(string filename, Ptr<aruco::DetectorParameters> params);
        bool writeDetectorParameters(string filename, Ptr<aruco::DetectorParameters> params);
        basic_ostream<char>& operator<<(basic_ostream<char>& outs, Ptr<DetectorParameters> params);
        basic_ostream<char>& operator<<(basic_ostream<char>& outs, SimpleBlobDetector::Params params);  
        void parseBlobParameters(const char * paramString, SimpleBlobDetector::Params *params);
        void parseDetectorParameters(const char * paramString, Ptr<DetectorParameters> &params);
    }
}

#endif
