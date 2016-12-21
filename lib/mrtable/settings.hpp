
#ifndef __PARSERS_HPP__
#define __PARSERS_HPP__

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

        bool readBlobParameters(string filename, SimpleBlobDetector::Params *params) {
            FileStorage fs(filename, FileStorage::READ);
            if (!fs.isOpened()) 
                return false;
            params->read(fs.getFirstTopLevelNode());
            /*
            fs["blobColor"] >> params->blobColor;
            fs["filterByArea"] >> params->filterByArea;
            fs["filterByCircularity"] >> params->filterByCircularity;
            fs["filterByColor"] >> params->filterByColor;
            fs["filterByConvexity"] >> params->filterByConvexity;
            fs["filterByInertia"] >> params->filterByInertia;
            fs["maxArea"] >> params->maxArea;
            fs["maxCircularity"] >> params->maxCircularity;
            fs["maxInertiaRatio"] >> params->maxInertiaRatio;
            fs["maxThreshold"] >> params->maxThreshold;
            fs["minArea"] >> params->minArea;
            fs["minCircularity"] >> params->minCircularity;
            fs["minInertiaRatio"] >> params->minInertiaRatio;
            fs["minThreshold"] >> params->minThreshold;
            fs["minDistBetweenBlobs"] >> params->minDistBetweenBlobs;
            int temp;
            fs["minRepeatability"] >> temp;
            params->minRepeatability = temp;
            fs["thresholdStep"] >> params->thresholdStep;
            */
            return true;
        }

        bool writeBlobParameters(string filename, SimpleBlobDetector::Params *params) {
            FileStorage fs(filename, FileStorage::WRITE);
            if(!fs.isOpened())
                return false;
            params->write(fs);
            /*
            fs << "blobColor" << params->blobColor;
            fs << "filterByArea" << params->filterByArea;
            fs << "filterByCircularity" << params->filterByCircularity;
            fs << "filterByColor" << params->filterByColor;
            fs << "filterByConvexity" << params->filterByConvexity;
            fs << "filterByInertia" << params->filterByInertia;
            fs << "maxArea" << params->maxArea;
            fs << "maxCircularity" << params->maxCircularity;
            fs << "maxInertiaRatio" << params->maxInertiaRatio;
            fs << "maxThreshold" << params->maxThreshold;
            fs << "minArea" << params->minArea;
            fs << "minCircularity" << params->minCircularity;
            fs << "minInertiaRatio" << params->minInertiaRatio;
            fs << "minThreshold" << params->minThreshold;
            fs << "minDistBetweenBlobs" << params->minDistBetweenBlobs;
            int temp = params->minRepeatability;
            fs << "minRepeatability" << temp; 
            fs << "thresholdStep" << params->thresholdStep;
            */
            fs.release();
            return true;
        }

        bool readDetectorParameters(string filename, Ptr<aruco::DetectorParameters> &params) {
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

        bool writeDetectorParameters(string filename, Ptr<aruco::DetectorParameters> params) {
            FileStorage fs(filename, FileStorage::WRITE);
            if(!fs.isOpened())
                return false;
            fs << "adaptiveThreshWinSizeMin" << params->adaptiveThreshWinSizeMin;
            fs << "adaptiveThreshWinSizeMax" << params->adaptiveThreshWinSizeMax;
            fs << "adaptiveThreshWinSizeStep" << params->adaptiveThreshWinSizeStep;
            fs << "adaptiveThreshConstant" << params->adaptiveThreshConstant;
            fs << "minMarkerPerimeterRate" << params->minMarkerPerimeterRate;
            fs << "maxMarkerPerimeterRate" << params->maxMarkerPerimeterRate;
            fs << "polygonalApproxAccuracyRate" << params->polygonalApproxAccuracyRate;
            fs << "minCornerDistanceRate" << params->minCornerDistanceRate;
            fs << "minDistanceToBorder" << params->minDistanceToBorder;
            fs << "minMarkerDistanceRate" << params->minMarkerDistanceRate;
            fs << "doCornerRefinement" << params->doCornerRefinement;
            fs << "cornerRefinementWinSize" << params->cornerRefinementWinSize;
            fs << "cornerRefinementMaxIterations" << params->cornerRefinementMaxIterations;
            fs << "cornerRefinementMinAccuracy" << params->cornerRefinementMinAccuracy;
            fs << "markerBorderBits" << params->markerBorderBits;
            fs << "perspectiveRemovePixelPerCell" << params->perspectiveRemovePixelPerCell;
            fs << "perspectiveRemoveIgnoredMarginPerCell" << params->perspectiveRemoveIgnoredMarginPerCell;
            fs << "maxErroneousBitsInBorderRate" << params->maxErroneousBitsInBorderRate;
            fs << "minOtsuStdDev" << params->minOtsuStdDev;
            fs << "errorCorrectionRate" << params->errorCorrectionRate;
            fs.release();
            return true;
        }

        void printBlobParameters(SimpleBlobDetector::Params *params) {
            cout << "blobColor: " << (unsigned int)params->blobColor << endl;
            cout << "filterByArea: " << (params->filterByArea ? "true" : "false") << endl;
            cout << "filterByCircularity: " << (params->filterByCircularity ? "true" : "false") << endl;
            cout << "filterByColor: " << (params->filterByColor ? "true" : "false") << endl;
            cout << "filterByConvexity: " << (params->filterByConvexity ? "true" : "false") << endl;
            cout << "filterByInertia: " << (params->filterByInertia ? "true" : "false") << endl;
            cout << "maxArea: " << params->maxArea << endl;
            cout << "maxCircularity: " << params->maxCircularity << endl;
            cout << "maxConvexity: " << params->maxConvexity << endl;
            cout << "maxInertiaRatio: " << params->maxInertiaRatio << endl;
            cout << "maxThreshold: " << params->maxThreshold << endl;
            cout << "minArea: " << params->minArea << endl;
            cout << "minCircularity: " << params->minCircularity << endl;
            cout << "minConvexity: " << params->minConvexity << endl;
            cout << "minDistBetweenBlobs: " << params->minDistBetweenBlobs << endl;
            cout << "minInertiaRatio: " << params->minInertiaRatio << endl;
            cout << "minRepeatability: " << params->minRepeatability << endl;
            cout << "minThreshold: " << params->minThreshold << endl;
            cout << "thresholdStep: " << params->thresholdStep << endl;
        }
   
       void printDetectorParameters(Ptr<DetectorParameters> params) {
            cout << "adaptiveThreshConstant: " << params->adaptiveThreshConstant << endl;
            cout << "adaptiveThreshWinSizeMax: " << params->adaptiveThreshWinSizeMax << endl;
            cout << "adaptiveThreshWinSizeMin: " << params->adaptiveThreshWinSizeMin << endl;
            cout << "adaptiveThreshWinSizeStep: " << params->adaptiveThreshWinSizeStep << endl;
            cout << "cornerRefinementMaxIterations: " << params->cornerRefinementMaxIterations << endl;
            cout << "cornerRefinementMinAccuracy: " << params->cornerRefinementMinAccuracy << endl;
            cout << "cornerRefinementWinSize: " << params->cornerRefinementWinSize << endl;
            cout << "errorCorrectionRate: " << params->errorCorrectionRate << endl;
            cout << "markerBorderBits: " << params->markerBorderBits << endl;
            cout << "maxErroneousBitsInBorderRate: " << params->maxErroneousBitsInBorderRate << endl;
            cout << "maxMarkerPerimeterRate: " << params->maxMarkerPerimeterRate << endl;
            cout << "minCornerDistanceRate: " << params->minCornerDistanceRate << endl;
            cout << "minDistanceToBorder: " << params->minDistanceToBorder << endl;
            cout << "minMarkerDistanceRate: " << params->minMarkerDistanceRate << endl;
            cout << "minMarkerPerimeterRate: " << params->minMarkerPerimeterRate << endl;
            cout << "minOtsuStdDev: " << params->minOtsuStdDev << endl;
            cout << "perspectiveRemoveIgnoredMarginPerCell: " << params->perspectiveRemoveIgnoredMarginPerCell << endl;
            cout << "perspectiveRemovePixelPerCell: " << params->perspectiveRemovePixelPerCell << endl;
            cout << "polygonalApproxAccuracyRate: " << params->polygonalApproxAccuracyRate << endl;
            cout << "doCornerRefinement: " << (params->doCornerRefinement ? "true" : "false") << endl;
        }

        void parseBlobParameters(const char * paramString, SimpleBlobDetector::Params *params) {
            string input = paramString;
            istringstream ss(input);
            string token;
            while (std::getline(ss, token, ',')) {
                istringstream pair(token);
                string key, value;
                if (std::getline(pair, key, '=') && std::getline(pair, value)) {
                    transform(key.begin(), key.end(), key.begin(), ::tolower);
                    transform(value.begin(), value.end(), value.begin(), ::tolower);
                    if (key.compare("blobcolor") == 0) {
                        params->blobColor = stoi(value);
                    }
                    if (key.compare("filterbyarea") == 0) {
                        if (value.compare("true") == 0) {
                            params->filterByArea = true;
                        }
                        if (value.compare("false") == 0) {
                            params->filterByArea = false;
                        }
                    }
                    if (key.compare("filterbycolor") == 0) {
                        if (value.compare("true") == 0) {
                            params->filterByColor = true;
                        }
                        if (value.compare("false") == 0) {
                            params->filterByColor = false;
                        }
                    }
                    if (key.compare("filterbyconvexity") == 0) {
                        if (value.compare("true") == 0) {
                            params->filterByConvexity = true;
                        }
                        if (value.compare("false") == 0) {
                            params->filterByConvexity = false;
                        }
                    }
                    if (key.compare("filterbyinertia") == 0) {
                         if (value.compare("true") == 0) {
                            params->filterByInertia = true;
                        }
                        if (value.compare("false") == 0) {
                            params->filterByInertia = false;
                        }
                    }
                    if (key.compare("maxarea") == 0) {
                        params->maxArea = stof(value);
                    }
                    if (key.compare("maxcircularity") == 0) {
                        params->maxCircularity = stoi(value);
                    }
                    if (key.compare("maxconvexity") == 0) {
                        params->maxConvexity = stof(value);
                    }
                    if (key.compare("maxinertiaRatio") == 0) {
                        params->maxInertiaRatio = stoi(value);
                    }
                    if (key.compare("maxthreshold") == 0) {
                        params->maxThreshold = stof(value);
                    }
                    if (key.compare("minarea") == 0) {
                        params->minArea = stof(value);
                    }
                    if (key.compare("mincircularity") == 0) {
                        params->minCircularity = stof(value);
                    }
                    if (key.compare("minconvexity") == 0) {
                        params->minConvexity = stof(value);
                    }
                    if (key.compare("mindistbetweenblobs") == 0) {
                        params->minDistBetweenBlobs = stoi(value);
                    }
                    if (key.compare("mininertiaratio") == 0) {
                        params->minInertiaRatio = stof(value);
                    }
                    if (key.compare("minrepeatability") == 0) {
                        params->minRepeatability = stoi(value);
                    }
                    if (key.compare("minthreshold") == 0) {
                        params->minInertiaRatio = stof(value);
                    }
                    if (key.compare("thresholdstep") == 0) {
                        params->minInertiaRatio = stof(value);
                    }
                    if (key.compare("mininertiaratio") == 0) {
                        params->minInertiaRatio = stof(value);
                    }
                }                
            }
        }

        void parseDetectorParameters(const char * paramString, Ptr<DetectorParameters> &params) {
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
