#include <opencv2/highgui/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <tclap/CmdLine.h>
#include <rapidxml/rapidxml.hpp>
#include <vector>
#include <iostream>
#include <chrono>
#include <string>
#include <fstream>
#include <streambuf>
#include <mrtable/settings.hpp>

#define FILETYPE_NONE 0
#define FILETYPE_IMAGE 1
#define FILETYPE_VIDEO 2
#define WINDOW_NAME "Test Markers"

using namespace cv;
using namespace std;
using namespace cv::aruco;
using namespace std::chrono;
using namespace TCLAP;
using namespace rapidxml;
using namespace mrtable::settings;

Mat cameraMatrix;
Mat distCoeffs;
bool bCameraSettings = false;
DetectorParameters parameters;

Dictionary dictionary = getPredefinedDictionary(DICT_4X4_100);
vector<int> markerIds;
vector< vector<Point2f> > markerCorners, rejectedCandidates;
bool bEstimatePose = false;

bool bOutputAll = false;
string outputPrefix;

bool verbose = false;
bool show = false;

typedef struct result_t {
    long elapsed;
    int detected;
} result_t;


int getFileType(char const * filename) {
    char* buf;
    int len = strlen(filename);
    for (int i = len; i >= 0; i--) {
        if (i == 0) {
            return FILETYPE_NONE;
        }
        if (filename[i] == '.') {
            buf = new char[len - i + 2];
            strcpy(buf, &filename[i]);
            break;
        }
    }
    if (strstr(".jpg.bmp.dib.jpeg.jpe.jp2.png.pbm.pgm.ppm.sr.ras.tif.tiff", buf)) {
        return FILETYPE_IMAGE;
    }
    if (strstr(".avi.mpg.mov", buf)) {
        return FILETYPE_VIDEO;
    }
    return FILETYPE_NONE;
}

result_t processImage(Mat input, Mat * output) {
    long elapsed = 0;
    steady_clock::time_point detectBegin = steady_clock::now();
    detectMarkers(input, dictionary, markerCorners, markerIds, parameters, rejectedCandidates);
    steady_clock::time_point detectEnd = steady_clock::now();
    elapsed = duration_cast<milliseconds>(detectEnd - detectBegin).count();
    if (output) {
        input.copyTo(*output);
    }
    if (bEstimatePose) {
        Mat cameraMatrix, distCoeffs;
        vector< Vec3d > rvecs, tvecs;
        steady_clock::time_point estimateBegin = steady_clock::now();
        estimatePoseSingleMarkers(markerCorners, 0.05, cameraMatrix, distCoeffs, rvecs, tvecs);
        steady_clock::time_point estimateEnd = steady_clock::now();
        elapsed += duration_cast<milliseconds>(estimateEnd - estimateBegin).count();
        vector< Vec3d >::iterator rvec = rvecs.begin();
        vector< Vec3d >::iterator tvec = tvecs.begin(); 
        if (output) {
            for (;rvec != rvecs.end() && tvec != tvecs.end(); ++rvec, ++tvec) {
                drawAxis(*output, cameraMatrix, distCoeffs, *rvec, *tvec, 0.05);
            }
        }
    }
    if (output) {
        drawDetectedMarkers(*output, markerCorners, markerIds);
    }
    result_t result;
    result.elapsed = elapsed;
    result.detected = markerIds.size();
    return result;
}

double processVideo() {
    // VideoCapture cap("testmovie.mov");
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cout << "Cannot open camera" << endl;
        return -1;
    }

    int frameCount = 0;
    int detectCount = 0;
    long start = time(0);
    bool bSuccess = true;
    char droppedString[3] = { 0 };
    while (bSuccess) {
        Mat frame;
        bSuccess = cap.read(frame); 
        if (!bSuccess) {
            break;
        }
        detectMarkers(frame, dictionary, markerCorners, markerIds, parameters, rejectedCandidates);
        drawDetectedMarkers(frame, markerCorners, markerIds);
        sprintf(droppedString, "%d", (int)markerIds.size());
        putText(frame, droppedString, Point2f(10, 500), FONT_HERSHEY_SIMPLEX, 4, Scalar(0, 0, 255, 0), 2, LINE_AA);
        
        frameCount++;
        detectCount += (int)markerIds.size();
        
        
        imshow("Cam", frame);

        if (waitKey(20) == 27) {
            cout << "Escape!" << endl;
            break;
        }
        
        
        bool bSuccess = cap.read(frame);
    }
    long finish = time(0);

    cout << "Average detections per frame: " << ((double)detectCount / frameCount) << endl;
    cout << "Frames per second: " << ((double)frameCount / (finish - start)) << endl;

    return (double)detectCount / frameCount;
}


int oldmain(int argc, char **argv) {
    /*
    parameters.adaptiveThreshWinSizeMin = 3;
    parameters.adaptiveThreshWinSizeMax = 118;
    parameters.adaptiveThreshWinSizeStep = 5;
    parameters.minMarkerDistanceRate = 0.01;
    // Should sweep adaptiveThreshConstant for different lighting
    parameters.adaptiveThreshConstant = 5;
    parameters.perspectiveRemoveIgnoredMarginPerCell = 0.25;
    parameters.perspectiveRemovePixelPerCell = 4;

    */
    /*
    while (1) {
        process();
    }
    */
    /*
    char filename[7] = { 0 };
    char num[3] = { 0 };
    strcpy(filename, "00.jpg");
    for (parameters.adaptiveThreshConstant = 1; parameters.adaptiveThreshConstant < 15; parameters.adaptiveThreshConstant++) {
        Mat output = singleImage();
        sprintf(filename, "%02d.jpg", (int)parameters.adaptiveThreshConstant);
        imwrite(filename, output);
    }
    */
    //process();
    return 0;
}

int main(int argc, char** argv) {
    try {
        // Setup TCLAP command line parser
        CmdLine cmd("Utility to test detection and pose estimation of Aruco markers", ' ', "0.1");

        // Add TCLAP switches and parameters
        ValueArg<string> inputfileArg("i", "inputfile", "Image or video file of Aruco markers for testing", false, "", "filename", cmd);
        ValueArg<string> cameraxmlArg("c", "camera", "XML file containing camera settings calibration data. Required for pose detection.", false, "", "filename", cmd);
        ValueArg<string> detectorxmlArg("d", "detector", "XML file containing marker detector parameters.", false, "", "filename", cmd);
        ValueArg<string> outputArg("o", "output", "Output camera settings, marker detector parameters and preview of images/video with superimposed detected markers/pose estimation axes using the specified filename prefix.", false, "output_", "filename prefix", cmd); 
        ValueArg<string> parametersArg("p", "parameters", "List of detector parameter settings, i.e. adaptiveThreshConstant=7.2,adaptiveThreshWinSizeMax=28,doCornerRefinement=false. Overrides default settings and settings specified in detector XML file.", false, "", "string", cmd);
        SwitchArg showSwitch("s", "show", "Show Aruco marker detection in a UI window", cmd, false);
        SwitchArg verboseSwitch("v", "verbose", "Verbose output of camera settings, distortion coefficients and detector parameters.", cmd, false);

        // Parse switches and parameters 
        cmd.parse(argc, argv);

        verbose = verboseSwitch.isSet();

        string cameraxml = cameraxmlArg.getValue();
        if (cameraxml.length() > 0) {
            if (verbose) {
                cout << "Parsing camera XML file: " << cameraxml << endl;
            }
            try {
                parseCameraSettings(cameraxml.c_str(), &cameraMatrix, &distCoeffs);
                bCameraSettings = true;
                bEstimatePose = true;
            } catch (const std::exception& e) {
                cout << "Exception reading Camera Settings XML file" << endl;
                return 1;
            }
        }

        string parameterxml = detectorxmlArg.getValue();
        if (parameterxml.length() > 0) {
            if (verbose) {
                cout << "Parsing detector parameter XML file: " << parameterxml << endl;
            }
            try {
                readDetectorParameters(parameterxml.c_str(), &parameters);
            } catch (const std::exception& e) {
                cout << "Exception reading Detector Parameter XML file" << endl;
                return 1;
            }
        }

        if (parametersArg.isSet()) {
            string params = parametersArg.getValue();
            parseDetectorParameters(params.c_str(), &parameters);
        }

        if (verbose) {
            cout << "Camera matrix: " << endl << cameraMatrix << endl;
            cout << "Distortion coefficients: " << endl << distCoeffs << endl;
            cout << "Marker detection parameters: " << endl;
            printDetectorParameters(parameters);
            cout << endl;
        }

        outputPrefix = outputArg.getValue();
        if (outputArg.isSet()) {
            bOutputAll = true;
            if (verbose) {
                cout << "Output camera settings, detector parameters, preview with prefix: " << outputPrefix << endl;
            }
            string camFile = outputPrefix + "cameraSettings.xml";
            string paramFile = outputPrefix + "detectorParameters.xml"; 
            writeCameraSettings(camFile.c_str(), cameraMatrix, distCoeffs);
            writeDetectorParameters(paramFile.c_str(), parameters);
        }

        show = showSwitch.isSet();
        if (show) {
            if (verbose) {
                cout << "Display Aruco marker detection in UI window" << endl;
            }
            namedWindow(WINDOW_NAME);
        }

        if (inputfileArg.isSet() > 0) {
            string inputfile = inputfileArg.getValue();
            if (verbose) {
                cout << "Input file: " << inputfile << endl; 
            }
            int filetype = getFileType(inputfile.c_str());
            if (filetype == FILETYPE_NONE) {
                cout << "Unrecognized input file extension" << endl;
                return 1;
            }
            if (filetype == FILETYPE_IMAGE) {
                Mat image = imread(inputfile);
                Mat output;
                result_t result;
                if (show || bOutputAll) {
                    result = processImage(image, &output);
                } else {
                    result = processImage(image, NULL);
                }
                cout << "milliseconds per frame: " << result.elapsed << endl;
                cout << "detected markers: " << result.detected << endl;
                if (bOutputAll) {
                    string outputName = outputPrefix;
                    for (int i = inputfile.length(); i >= 0; i--) {
                        if (inputfile[i] == '/') {
                            outputName += inputfile.substr(i + 1);
                            break;
                        }
                        if (i == 0) {
                            outputName += inputfile;
                        }
                    }
                    if (verbose) {
                        cout << "Writing image file: " << outputName << endl;
                    }
                    imwrite(outputName, output);
                }
                if (show) {
                    cout << "Press any key to continue..." << endl;
                    imshow(WINDOW_NAME, output);
                    waitKey(0);
                }
            }
        }


    } catch (TCLAP::ArgException &e) {
        cerr << "error: " << e.error() << " for argument " << e.argId() << endl;
        return 1;
    }
    return 0;
}
