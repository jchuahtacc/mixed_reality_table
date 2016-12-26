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
#include <opencv2/features2d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <mrtable/settings.hpp>
#include <chrono>

using namespace std;
using namespace cv;
using namespace mrtable::settings;
using namespace std::chrono;

namespace {
const char* about = "Basic blob detection";
const char* keys  =
        "{h        |       | Print this help message }"
        "{v        |       | Input from video file, if ommited, input comes from camera }"
        "{i        |       | Input from a single image file }"
        "{ci       | 0     | Camera id if input doesnt come from video (-v) }"
        "{cp       |       | File of blob detector parameters }"
        "{op       |       | List of overridden blob detector parameters, i.e. minPointRatios=0.3,minRadiusRatio=0.1}"
        "{o        |       | Output camera and detector parameters and preview renderings. File prefix is output_}"
        "{p        |       | Show preview in GUI window }"
        "{n        |       | Do not create a negative before detection}"
        "{verbose  |       | Verbose output of settings }";
}

typedef struct result_t {
    long int frames;
    long int detected;
    long int elapsed;
} result_t;

bool preview;
bool output;
bool verbose;
bool nonnegative;

ContourParams params;
vector< KeyPoint > keypoints;
long imgSize = 1;
Mat temp;

void outputResults(result_t result) {
    cout << "Frames processed: " << result.frames << endl;
    cout << "Milliseconds elapsed: " << result.elapsed << endl;
    cout << "Touches detected: " << result.detected << endl;
    cout << "Average frames per second: " << (result.frames * 1000.0 / result.elapsed) << endl;
    cout << "Average blobs detected per frame: " << (result.detected * 1.0 / result.frames) << endl;
}

string getFilename(string filename) {
    for (int i = filename.length(); i >= 0; i--) {
        if (filename[i] == '/') {
            return filename.substr(i + 1);
        }
    }
    return filename;
}

result_t processImage(Mat image, Mat* imageCopy) {
    steady_clock::time_point detectBegin = steady_clock::now();

    // detect markers and estimate pose
    cvtColor(image, temp, CV_BGR2GRAY);
    if (!nonnegative) {
        bitwise_not(temp, temp);
    }
    threshold(temp, temp, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(temp, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE);
    vector< Point2f > centers;
    vector< float > radii;
    for (int i = 0; i < contours.size(); i++) {
        double ratio = (double)contours[i].size() / imgSize; 
        if (ratio > params.minPointRatio && ratio < params.maxPointRatio ) {
            if (verbose) {
                cout << "Detected contour with point to size ratio: " << ratio << endl; 
            }
            Point2f center;
            float radius;
            minEnclosingCircle(contours[i], center, radius);
            if (radius > params.minRadiusRatio && radius < params.maxRadiusRatio) {
                centers.push_back(center);
                radii.push_back(radius);
                if (verbose) {
                    cout << "Contour radius within parameters: " << radius << endl;
                }
            } else {
                if (verbose) {
                    cout << "Discarded contour with radius: " << radius << endl;
                }
            }
        } else {
            if (verbose) {
                cout << "Discarding contour with point to size ratio: " << ratio << endl; 
            }
        }
    }

    steady_clock::time_point detectEnd = steady_clock::now();
    long elapsed = duration_cast<milliseconds>(detectEnd - detectBegin).count();
 
    // draw results
    if (imageCopy) {
        temp.copyTo(*imageCopy);
        for (int i = 0; i < centers.size(); i++) {
            circle(*imageCopy, centers[i], (int)radii[i], Scalar(0, 0, 255), 5);
        }
    }

    result_t result;
    result.frames = 1;
    result.elapsed = elapsed;
    result.detected = centers.size();

    return result;
}

/**
 */
int main(int argc, char *argv[]) {
    CommandLineParser parser(argc, argv, keys);
    parser.about(about);

    if (parser.has("h")) {
        parser.printMessage();
        return 0;
    }

    if(argc < 0) {
        parser.printMessage();
        return 0;
    }

    int camId = parser.get<int>("ci");

    preview = parser.has("p");
    output = parser.has("o");
    verbose = parser.has("verbose");
    nonnegative = parser.has("n");
    String overrideParameters = parser.get<String>("op");

    if(parser.has("cp")) {
        bool readOk = readContourParameters(parser.get<string>("cp"), &params);
        if(!readOk) {
            cerr << "Invalid contour detector parameters file" << endl;
            return 0;
        }
    }

    if(!parser.check()) {
        parser.printErrors();
        return 0;
    }

    if (!overrideParameters.empty()) {
        parseContourParameters(overrideParameters.c_str(), &params);
    }

    if (verbose) {
        printContourParameters(params);
    }

    if(output) {
        writeContourParameters("output_contourParams.xml", params);
    }

    if (parser.has("i")) {
        string filename = parser.get<String>("i");
        Mat image, imageCopy;
        image = imread(filename);
        imgSize = (image.rows + image.cols) / 2;
        result_t result = processImage(image, &imageCopy);
        if (output) {
            imwrite("output_" + getFilename(filename), imageCopy); 
        }
        if (preview) {
            cout << "Press the Escape key to continue..." << endl;
            imshow("out", imageCopy);
            char key = (char)waitKey(0);
        }
        outputResults(result);
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

    imgSize = (inputVideo.get(CV_CAP_PROP_FRAME_WIDTH) + inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT)) / 2;
    VideoWriter outputVideo;

    // Disable camera capture -- can't get it working
    if (!videoFile && output) {
        output = false;
    }
    if (output) {
        int ex;
        Size S = Size((int) inputVideo.get(CV_CAP_PROP_FRAME_WIDTH), (int) inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT));
 
        string outputFilename;
        if (!videoFile) {
            ex = CV_FOURCC('H','2','6','3');
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
            result = processImage(image, &imageCopy);
        } else {
            result = processImage(image, NULL);
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
