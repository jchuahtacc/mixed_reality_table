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

Mat processImage(Mat input) {
    // Mat image;
    Mat output;
    // image = imread("IMAG1277-contrast.jpg");
    detectMarkers(input, dictionary, markerCorners, markerIds, parameters, rejectedCandidates);
    input.copyTo(output);
    if (bEstimatePose) {
        Mat cameraMatrix, distCoeffs;
        vector< Vec3d > rvecs, tvecs;
        estimatePoseSingleMarkers(markerCorners, 0.05, cameraMatrix, distCoeffs, rvecs, tvecs);
        vector< Vec3d >::iterator rvec = rvecs.begin();
        vector< Vec3d >::iterator tvec = tvecs.begin(); 
        for (;rvec != rvecs.end() && tvec != tvecs.end(); ++rvec, ++tvec) {
            drawAxis(output, cameraMatrix, distCoeffs, *rvec, *tvec, 0.05);
        }
    }
    drawDetectedMarkers(output, markerCorners, markerIds);
    return output;
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

        string inputfile = inputfileArg.getValue();
        if (inputfile.length() > 0) {
            cout << "Input file: " << inputfile << endl; 
        }

        string cameraxml = cameraxmlArg.getValue();
        if (cameraxml.length() > 0) {
            cout << "Parsing camera XML file: " << cameraxml << endl;
            try {
                parseCameraSettings(cameraxml.c_str(), &cameraMatrix, &distCoeffs);
                bCameraSettings = true;
                cout << "cameraMatrix: " << endl << cameraMatrix << endl;
                cout << "distCoeffs: " << endl << distCoeffs << endl;
            } catch (const std::exception& e) {
                cout << "Exception reading Camera Settings XML file" << endl;
                return 1;
            }
        }

        string parameterxml = detectorxmlArg.getValue();
        if (parameterxml.length() > 0) {
            cout << "Parsing detector parameter XML file: " << parameterxml << endl;
            try {
                readDetectorParameters(parameterxml.c_str(), &parameters);
                printDetectorParameters(parameters);
            } catch (const std::exception& e) {
                cout << "Exception reading Detector Parameter XML file" << endl;
                return 1;
            }
        }

        if (parametersArg.isSet()) {
            string params = parametersArg.getValue();
            parseDetectorParameters(params.c_str(), &parameters);
        }

        if (verboseSwitch.getValue()) {
            cout << "Camera matrix: " << endl << cameraMatrix << endl;
            cout << "Distortion coefficients: " << endl << distCoeffs << endl;
            cout << "Marker detection parameters: " << endl;
            printDetectorParameters(parameters);
            cout << endl;
        }

        outputPrefix = outputArg.getValue();
        if (outputArg.isSet()) {
            bOutputAll = true;
            cout << "Output camera settings, detector parameters, preview with prefix: " << outputPrefix << endl;
            string camFile = outputPrefix + "cameraSettings.xml";
            string paramFile = outputPrefix + "detectorParameters.xml"; 
            writeCameraSettings(camFile.c_str(), cameraMatrix, distCoeffs);
            writeDetectorParameters(paramFile.c_str(), parameters);
        }

        if (showSwitch.getValue()) {
            cout << "Display Aruco marker detection in UI window" << endl;
        }
    } catch (TCLAP::ArgException &e) {
        cerr << "error: " << e.error() << " for argument " << e.argId() << endl;
        return 1;
    }
    return 0;
}
