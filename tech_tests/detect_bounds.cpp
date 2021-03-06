/*
By downloading, copying, installing or using the software you agree to this
license. If you do not agree to this license, do not download, install,
copy or use the software.
                          License Agreement
               For Open Source Computer Vision Library
                       (3-clause BSD License)
Copyright (C) 2013, OpenCV Foundation, all rights reserved.
Third party copyrights are property of their respective owners.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  * Neither the names of the copyright holders nor the names of the contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.
This software is provided by the copyright holders and contributors "as is" and
any express or implied warranties, including, but not limited to, the implied
warranties of merchantability and fitness for a particular purpose are
disclaimed. In no event shall copyright holders or contributors be liable for
any direct, indirect, incidental, special, exemplary, or consequential damages
(including, but not limited to, procurement of substitute goods or services;
loss of use, data, or profits; or business interruption) however caused
and on any theory of liability, whether in contract, strict liability,
or tort (including negligence or otherwise) arising in any way out of
the use of this software, even if advised of the possibility of such damage.
*/


#include <opencv2/highgui/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <iostream>
#include <mrtable/mrtable.hpp>
#include <chrono>

using namespace std;
using namespace cv;
using namespace mrtable::config;
using namespace std::chrono;

namespace {
const char* about = "Basic marker detection";
const char* keys  =
        "{h        |       | Print this help message}"
        "{d        |       | dictionary: DICT_4X4_50=0, DICT_4X4_100=1, DICT_4X4_250=2,"
        "DICT_4X4_1000=3, DICT_5X5_50=4, DICT_5X5_100=5, DICT_5X5_250=6, DICT_5X5_1000=7, "
        "DICT_6X6_50=8, DICT_6X6_100=9, DICT_6X6_250=10, DICT_6X6_1000=11, DICT_7X7_50=12,"
        "DICT_7X7_100=13, DICT_7X7_250=14, DICT_7X7_1000=15, DICT_ARUCO_ORIGINAL = 16}"
        "{v        |       | Input from video file, if ommited, input comes from camera }"
        "{i        |       | Input from a single image file }"
        "{ci       | 0     | Camera id if input doesnt come from video (-v) }"
        "{dp       |       | File of marker detector parameters }"
        "{op       |       | List of overridden detector parameters, i.e. adaptiveThreshConstant=5,doCornerRefinement=false}"
        "{o        |       | Output camera and detector parameters and preview renderings. File prefix is output_}"
        "{p        |       | Show preview in GUI window }"
        "{verbose  |       | Verbose output of settings }"
        "{r        |       | show rejected candidates too }";
}

int dictionaryId;
bool showRejected;
bool estimatePose;
bool preview;
bool output;
bool verbose;
float markerLength;

Ptr<aruco::DetectorParameters> detectorParams = aruco::DetectorParameters::create();
Ptr<aruco::Dictionary> dictionary;
Mat camMatrix, distCoeffs;
vector< int > ids;
vector< vector< Point2f > > corners, rejected;
vector< Vec3d > rvecs, tvecs;

void outputResults(result_t result) {
    cout << "Frames processed: " << result.frames << endl;
    cout << "Milliseconds elapsed: " << result.elapsed << endl;
    cout << "Markers detected: " << result.detected << endl;
    cout << "Average frames per second: " << (result.frames * 1000.0 / result.elapsed) << endl;
    cout << "Average markers detected per frame: " << (result.detected * 1.0 / result.frames) << endl;
}

string getFilename(string filename) {
    for (int i = filename.length(); i >= 0; i--) {
        if (filename[i] == '/') {
            return filename.substr(i + 1);
        }
    }
    return filename;
}


/**
 */
int main(int argc, char *argv[]) {
    CommandLineParser parser(argc, argv, keys);
    parser.about(about);

    if(parser.has("h")) {
        parser.printMessage();
        return 0;
    }

    dictionaryId = parser.get<int>("d");
    preview = parser.has("p");
    output = parser.has("o");
    verbose = parser.has("verbose");
    String overrideParameters = parser.get<String>("op");

    if(parser.has("dp")) {
        bool readOk = readDetectorParameters(parser.get<string>("dp"), detectorParams);
        if(!readOk) {
            cerr << "Invalid detector parameters file" << endl;
            return 0;
        }
    }

    int camId = 0;
    camId = parser.get<int>("ci");

    if(!parser.check()) {
        parser.printErrors();
        return 0;
    }

    dictionary = getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));

    if(estimatePose) {
        bool readOk = readCameraParameters(parser.get<string>("c"), camMatrix, distCoeffs);
        if(!readOk) {
            cerr << "Invalid camera file" << endl;
            return 0;
        }
    }

    if (!overrideParameters.empty()) {
        parseDetectorParameters(overrideParameters.c_str(), detectorParams);
    }

    if(output) {
        writeCameraParameters("output_cameraParams.xml", camMatrix, distCoeffs);
        writeDetectorParameters("output_detectorParams.xml", detectorParams);
    }

    if (parser.has("i")) {
        string filename = parser.get<String>("i");
        Mat image, imageCopy;
        image = imread(filename);
        cout << "Read image, detecting bounds" << endl;
        if (DetectBounds::verifyMarkerPlacement(image, dictionary, detectorParams)) {
            cout << "Marker placement OK " << endl;
            DetectBounds::calculateRoi();
            DetectBounds::dump(std::cout);
            /*
            int x = 300;
            int y = 100;
            if (DetectBounds::rot == 0) {
                cout << DetectBounds::getScreenPosition(Point(x, y)) << endl;
            }
            if (DetectBounds::rot == 1) {
                cout << DetectBounds::getScreenPosition(Point(DetectBounds::width - y, x)) << endl;
            }
            if (DetectBounds::rot == 2) {
                cout << DetectBounds::getScreenPosition(Point(DetectBounds::width - x, DetectBounds::height - y)) << endl;
            }
            if (DetectBounds::rot == 3) {
                cout << DetectBounds::getScreenPosition(Point(y, DetectBounds::height - x)) << endl;
            }
            */
        } else {
            cout << "*** Failed to detect bouds ***" << endl;
        }
        if (output) {
            imwrite("output_" + getFilename(filename), imageCopy); 
        }
        if (preview) {
            cout << "Press the Escape key to continue..." << endl;
            imshow("out", imageCopy);
            char key = (char)waitKey(0);
        }
        return 0;
    }

    String video;
    if(parser.has("v")) {
        video = parser.get<String>("v");
    }

    VideoCapture inputVideo;
    int waitTime;
    bool videoFile = false;
    if(!video.empty()) {
        videoFile = true;
        inputVideo.open(video);
        waitTime = 0;
    } else {
        inputVideo.open(camId);
        waitTime = 10;
    }

    VideoWriter outputVideo;

    // Disable camera capture -- can't get it working
    if (!videoFile && output) {
//        output = false;
    }
    if (output) {
        int ex;
        Size S = Size((int) inputVideo.get(CV_CAP_PROP_FRAME_WIDTH), (int) inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT));
 
        string outputFilename;
        if (!videoFile) {
            ex = CV_FOURCC('M','J','P','G');
            std::ostringstream oss;
            oss << "camera_" << camId << ".avi";
            outputFilename = "output_" + oss.str(); 
        } else {
            ex = static_cast<int>(inputVideo.get(CV_CAP_PROP_FOURCC));
            outputFilename = "output_" + getFilename(video);
        }
        outputVideo.open(outputFilename, ex, inputVideo.get(CV_CAP_PROP_FPS), S, true);
        if (!outputVideo.isOpened()) {
            cout << "Unable to open video file for output: " << ("output_" + getFilename(video)) << endl;
            return -1;
        }
    }

    result_t aggregate;
    aggregate.frames = 0;
    aggregate.detected = 0;
    aggregate.elapsed = 0;

    if (preview) {
        cout << "Press the Escape key to continue..." << endl;
    }


    while(inputVideo.grab()) {
        Mat image, imageCopy;
        inputVideo.retrieve(image);
        
        result_t result;

        if (preview || output) {
        //    result = processImage(image, &imageCopy);
        } else {
        //    result = processImage(image, NULL);
        }

        aggregate.frames++;
        aggregate.detected += result.detected;
        aggregate.elapsed += result.elapsed;

        if (output) {
            outputVideo << imageCopy;
        }

        if (preview) {
            imshow("out", imageCopy);
            char key = (char)waitKey(waitTime);
            if(key == 27) break;
        }
    }

    outputResults(aggregate);

    return 0;
}
