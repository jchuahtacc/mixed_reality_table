
#ifndef __PARSERS_HPP__
#define __PARSERS_HPP__

#include <iostream>
#include <vector>
#include <opencv2/aruco.hpp>
#include <opencv2/core/core.hpp>
#include <fstream>
#include <streambuf>
#include <string>
#include <sstream>

using namespace std;
using namespace cv;
using namespace cv::aruco;

namespace mrtable {
    namespace settings {

        bool readCameraParameters(string filename, Mat &camMatrix, Mat &distCoeffs) {
            FileStorage fs(filename, FileStorage::READ);
            if(!fs.isOpened())
                return false;
            fs["camera_matrix"] >> camMatrix;
            fs["distortion_coefficients"] >> distCoeffs;
            return true;
        }

        bool writeCameraParameters(string filename, Mat camMatrix, Mat distCoeffs) {
            FileStorage fs(filename, FileStorage::WRITE);
            if (!fs.isOpened())
                return false;
            fs << "camera_matrix" << camMatrix;
            fs << "distortion_coefficients" << distCoeffs;
            fs.release();
            return true;
        }

        bool readDetectorParameters(string filename, aruco::DetectorParameters *params) {
            FileStorage fs(filename, FileStorage::READ);
            if(!fs.isOpened())
                return false;
            fs["adaptiveThreshWinSizeMin"] >> params->adaptiveThreshWinSizeMin;
            fs["adaptiveThreshWinSizeMax"] >> params->adaptiveThreshWinSizeMax;
            fs["adaptiveThreshWinSizeStep"] >> params->adaptiveThreshWinSizeStep;
            fs["adaptiveThreshConstant"] >> params->adaptiveThreshConstant;
            fs["minMarkerPerimeterRate"] >> params->minMarkerPerimeterRate;
            fs["maxMarkerPerimeterRate"] >> params->maxMarkerPerimeterRate;
            fs["polygonalApproxAccuracyRate"] >> params->polygonalApproxAccuracyRate;
            fs["minCornerDistanceRate"] >> params->minCornerDistanceRate;
            fs["minDistanceToBorder"] >> params->minDistanceToBorder;
            fs["minMarkerDistanceRate"] >> params->minMarkerDistanceRate;
            fs["doCornerRefinement"] >> params->doCornerRefinement;
            fs["cornerRefinementWinSize"] >> params->cornerRefinementWinSize;
            fs["cornerRefinementMaxIterations"] >> params->cornerRefinementMaxIterations;
            fs["cornerRefinementMinAccuracy"] >> params->cornerRefinementMinAccuracy;
            fs["markerBorderBits"] >> params->markerBorderBits;
            fs["perspectiveRemovePixelPerCell"] >> params->perspectiveRemovePixelPerCell;
            fs["perspectiveRemoveIgnoredMarginPerCell"] >> params->perspectiveRemoveIgnoredMarginPerCell;
            fs["maxErroneousBitsInBorderRate"] >> params->maxErroneousBitsInBorderRate;
            fs["minOtsuStdDev"] >> params->minOtsuStdDev;
            fs["errorCorrectionRate"] >> params->errorCorrectionRate;
            return true;
        }

        bool writeDetectorParameters(string filename, aruco::DetectorParameters params) {
            FileStorage fs(filename, FileStorage::WRITE);
            if(!fs.isOpened())
                return false;
            fs << "adaptiveThreshWinSizeMin" << params.adaptiveThreshWinSizeMin;
            fs << "adaptiveThreshWinSizeMax" << params.adaptiveThreshWinSizeMax;
            fs << "adaptiveThreshWinSizeStep" << params.adaptiveThreshWinSizeStep;
            fs << "adaptiveThreshConstant" << params.adaptiveThreshConstant;
            fs << "minMarkerPerimeterRate" << params.minMarkerPerimeterRate;
            fs << "maxMarkerPerimeterRate" << params.maxMarkerPerimeterRate;
            fs << "polygonalApproxAccuracyRate" << params.polygonalApproxAccuracyRate;
            fs << "minCornerDistanceRate" << params.minCornerDistanceRate;
            fs << "minDistanceToBorder" << params.minDistanceToBorder;
            fs << "minMarkerDistanceRate" << params.minMarkerDistanceRate;
            fs << "doCornerRefinement" << params.doCornerRefinement;
            fs << "cornerRefinementWinSize" << params.cornerRefinementWinSize;
            fs << "cornerRefinementMaxIterations" << params.cornerRefinementMaxIterations;
            fs << "cornerRefinementMinAccuracy" << params.cornerRefinementMinAccuracy;
            fs << "markerBorderBits" << params.markerBorderBits;
            fs << "perspectiveRemovePixelPerCell" << params.perspectiveRemovePixelPerCell;
            fs << "perspectiveRemoveIgnoredMarginPerCell" << params.perspectiveRemoveIgnoredMarginPerCell;
            fs << "maxErroneousBitsInBorderRate" << params.maxErroneousBitsInBorderRate;
            fs << "minOtsuStdDev" << params.minOtsuStdDev;
            fs << "errorCorrectionRate" << params.errorCorrectionRate;
            fs.release();
            return true;
        }
    
       void printDetectorParameters(DetectorParameters params) {
            cout << "adaptiveThreshConstant: " << params.adaptiveThreshConstant << endl;
            cout << "adaptiveThreshWinSizeMax: " << params.adaptiveThreshWinSizeMax << endl;
            cout << "adaptiveThreshWinSizeMin: " << params.adaptiveThreshWinSizeMin << endl;
            cout << "adaptiveThreshWinSizeStep: " << params.adaptiveThreshWinSizeStep << endl;
            cout << "cornerRefinementMaxIterations: " << params.cornerRefinementMaxIterations << endl;
            cout << "cornerRefinementMinAccuracy: " << params.cornerRefinementMinAccuracy << endl;
            cout << "cornerRefinementWinSize: " << params.cornerRefinementWinSize << endl;
            cout << "errorCorrectionRate: " << params.errorCorrectionRate << endl;
            cout << "markerBorderBits: " << params.markerBorderBits << endl;
            cout << "maxErroneousBitsInBorderRate: " << params.maxErroneousBitsInBorderRate << endl;
            cout << "maxMarkerPerimeterRate: " << params.maxMarkerPerimeterRate << endl;
            cout << "minCornerDistanceRate: " << params.minCornerDistanceRate << endl;
            cout << "minDistanceToBorder: " << params.minDistanceToBorder << endl;
            cout << "minMarkerDistanceRate: " << params.minMarkerDistanceRate << endl;
            cout << "minMarkerPerimeterRate: " << params.minMarkerPerimeterRate << endl;
            cout << "minOtsuStdDev: " << params.minOtsuStdDev << endl;
            cout << "perspectiveRemoveIgnoredMarginPerCell: " << params.perspectiveRemoveIgnoredMarginPerCell << endl;
            cout << "perspectiveRemovePixelPerCell: " << params.perspectiveRemovePixelPerCell << endl;
            cout << "polygonalApproxAccuracyRate: " << params.polygonalApproxAccuracyRate << endl;
            cout << "doCornerRefinement: " << (params.doCornerRefinement ? "true" : "false") << endl;
        }

        void parseDetectorParameters(const char * paramString, DetectorParameters *params) {
            string input = paramString;
            istringstream ss(input);
            string token;
            while (std::getline(ss, token, ',')) {
                istringstream pair(token);
                string key, value;
                if (std::getline(pair, key, '=') && std::getline(pair, value)) {
                    transform(key.begin(), key.end(), key.begin(), ::tolower);
                    transform(value.begin(), value.end(), value.begin(), ::tolower);
                    if (key.compare("adaptivethreshconstant") == 0) {
                        params->adaptiveThreshConstant = stof(value);
                    }
                    if (key.compare("adaptivethreshwinsizemax") == 0) {
                        params->adaptiveThreshWinSizeMax = stoi(value);
                    }
                    if (key.compare("adaptivethreshwinsizemin") == 0) {
                        params->adaptiveThreshWinSizeMin = stoi(value);
                    }
                    if (key.compare("adaptivethreshwinsizestep") == 0) {
                        params->adaptiveThreshWinSizeStep = stoi(value);
                    }
                    if (key.compare("cornerrefinementmaxiterations") == 0) {
                        params->cornerRefinementMaxIterations = stoi(value);
                    }
                    if (key.compare("cornerrefinementminaccuracy") == 0) {
                        params->cornerRefinementMinAccuracy = stof(value);
                    }
                    if (key.compare("cornerrefinementwinsize") == 0) {
                        params->cornerRefinementWinSize = stoi(value);
                    }
                    if (key.compare("errorcorrectionrate") == 0) {
                        params->errorCorrectionRate = stof(value);
                    }
                    if (key.compare("markerborderbits") == 0) {
                        params->markerBorderBits = stoi(value);
                    }
                    if (key.compare("maxerroneousbitsinborderrate") == 0) {
                        params->maxErroneousBitsInBorderRate = stof(value);
                    }
                    if (key.compare("maxmarkerperimeterrate") == 0) {
                        params->maxMarkerPerimeterRate = stof(value);
                    }
                    if (key.compare("minotsustddev") == 0) {
                        params->minOtsuStdDev = stof(value);
                    }
                    if (key.compare("perspectiveremoveignoredmarginpercell") == 0) {
                        params->perspectiveRemoveIgnoredMarginPerCell = stof(value);
                    }
                    if (key.compare("perspectiveremovepixelpercell") == 0) {
                        params->perspectiveRemovePixelPerCell = stoi(value);
                    }
                    if (key.compare("polygonalapproxaccuracyrate") == 0) {
                        params->polygonalApproxAccuracyRate = stof(value);
                    }
                    if (key.compare("docornerrefinement") == 0) {
                        if (value.compare("true") == 0) {
                            params->doCornerRefinement = true;
                        }
                        if (value.compare("false") == 0) {
                            params->doCornerRefinement = false;
                        }
                    }
                }                
            }
        }
    }
}

#endif
