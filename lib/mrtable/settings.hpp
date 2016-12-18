
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
using namespace rapidxml;

namespace mrtable {
    namespace settings {
       
        void writeCameraSettings(const char * filename, Mat cameraMatrix, Mat distCoeffs) throw() {
            FileStorage fs(filename, FileStorage::WRITE);
            fs << "cameraMatrix" << cameraMatrix << "distCoeffs" << distCoeffs;
            fs.release();
        }

        void parseCameraSettings(const char * filename, Mat *cameraMatrix, Mat *distCoeffs) throw() {
            FileStorage fs(filename, FileStorage::READ);
            Mat cameraMatrix2, distCoeffs2;
            fs["cameraMatrix"] >> cameraMatrix2;
            fs["distCoeffs"] >> distCoeffs2;
            cameraMatrix2.copyTo(*cameraMatrix);
            distCoeffs2.copyTo(*distCoeffs);
            fs.release();
        }

        double getVal(xml_node<char> * node, const char * child) throw(std::runtime_error) {
            xml_node<char> *child_node = node->first_node(child);
            if (!child_node) {
                string mesg = "Node not found: ";
                mesg += child;
                throw (std::runtime_error(mesg.c_str()));
            }
            string contents = string(child_node->value());
            return stod(contents);
        }

        void readDetectorParameters(const char * filename, DetectorParameters* params) throw() {
            // Load xml file contents
            ifstream file;
            file.exceptions( ifstream::failbit | ifstream::badbit );
            file.open (filename);
            string contents;
            file.seekg(0, std::ios::end);
            contents.reserve(file.tellg());
            file.seekg(0, std::ios::beg);
            contents.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

            // Create XML document object
            xml_document<> doc;
            char* contents_buf = new char[(int)contents.length() + 1];
            strcpy(contents_buf, contents.c_str());
            doc.parse<0>(contents_buf);

            // Validate format 
            xml_node<char> *root = doc.first_node("DetectorParameters", 0, false);
            if (!root) {
                throw std::runtime_error("Missing DetectorParameters node");
            }

            // Get DetectorParameter settings
            try {
                params->adaptiveThreshConstant = getVal(root, "adaptiveThreshConstant");
            } catch (const std::runtime_error& er) {
            }
            try {
                params->adaptiveThreshWinSizeMax  = (int)getVal(root, "adaptiveThreshWinSizeMax");
            } catch (const std::runtime_error& er) {
            }
            try {
                params->adaptiveThreshWinSizeMin  = (int)getVal(root, "adaptiveThreshWinSizeMin");
            } catch (const std::runtime_error& er) {
            }
            try {
                params->adaptiveThreshWinSizeStep  = (int)getVal(root, "adaptiveThreshWinSizeStep");
            } catch (const std::runtime_error& er) {
            }
            try {
                params->cornerRefinementMaxIterations  = (int)getVal(root, "cornerRefinementMaxIterations");
            } catch (const std::runtime_error& er) {
            }
            try {
                params->cornerRefinementMinAccuracy = getVal(root, "cornerRefinementMinAccuracy");
            } catch (const std::runtime_error& er) {
            }
            try {
                params->cornerRefinementWinSize = (int)getVal(root, "cornerRefinementWinSize");
            } catch (const std::runtime_error& er) {
            } 
            try {
                params->markerBorderBits = (int)getVal(root, "markerBorderBits");
            } catch (const std::runtime_error& er) {
            } 
            try {
                params->maxErroneousBitsInBorderRate = getVal(root, "maxErroneousBitsInBorderRate");
            } catch (const std::runtime_error& er) {
            } 
            try {
                params->maxMarkerPerimeterRate = getVal(root, "maxMarkerPerimeterRate");
            } catch (const std::runtime_error& er) {
            } 
            try {
                params->minCornerDistanceRate = getVal(root, "minCornerDistanceRate");
            } catch (const std::runtime_error& er) {
            }
            try {
                params->minDistanceToBorder = (int)getVal(root, "minDistanceToBorder");
            } catch (const std::runtime_error& er) {
            }
            try {
                params->minMarkerDistanceRate = getVal(root, "minMarkerDistanceRate");
            } catch (const std::runtime_error& er) {
            }
            try {
                params->minOtsuStdDev = getVal(root, "minOtsuStdDev");
            } catch (const std::runtime_error& er) {
            }
            try {
                params->perspectiveRemoveIgnoredMarginPerCell = getVal(root, "perspectiveRemoveIgnoredMarginPerCell");
            } catch (const std::runtime_error& er) {
            }
            try {
                params->perspectiveRemovePixelPerCell = (int)getVal(root, "perspectiveRemovePixelPerCell");
            } catch (const std::runtime_error& er) {
            }
            try {
                params->polygonalApproxAccuracyRate = getVal(root, "polygonalApproxAccuracyRate");
            } catch (const std::runtime_error& er) {
            }
            xml_node< char > * cornerRefinementNode = root->first_node("doCornerRefinement");
            if (cornerRefinementNode) {
                char * contents = cornerRefinementNode->value();
                if (strcasecmp(contents, "false") == 0) {
                    params->doCornerRefinement = false;
                }
                if (strcasecmp(contents, "true") == 0) {
                    params->doCornerRefinement = true;
                }
            }
        }

        void writeDetectorParameters(const char * filename, DetectorParameters params) {
            ofstream file(filename);
            file << "<?xml version=\"1.0\" standalone=\"yes\" ?>" << endl;
            file << "<DetectorParameters>" << endl;
            file << "<adaptiveThreshConstant>" << params.adaptiveThreshConstant << "</adaptiveThreshConstant>" << endl;
            file << "<adaptiveThreshWinSizeMax>" << params.adaptiveThreshWinSizeMax << "</adaptiveThreshWinSizeMax>" << endl;
            file << "<adaptiveThreshWinSizeMin>" << params.adaptiveThreshWinSizeMin << "</adaptiveThreshWinSizeMin>" << endl;
            file << "<adaptiveThreshWinSizeStep>" << params.adaptiveThreshWinSizeStep << "</adaptiveThreshWinSizeStep>" << endl;
            file << "<cornerRefinementMaxIterations>" << params.cornerRefinementMaxIterations << "</cornerRefinementMaxIterations>" << endl;
            file << "<cornerRefinementMinAccuracy>" << params.cornerRefinementMinAccuracy << "</cornerRefinementMinAccuracy>" << endl;
            file << "<cornerRefinementWinSize>" << params.cornerRefinementWinSize << "</cornerRefinementWinSize>" << endl;
            file << "<errorCorrectionRate>" << params.errorCorrectionRate << "</errorCorrectionRate>" << endl;
            file << "<markerBorderBits>" << params.markerBorderBits << "</markerBorderBits>" << endl;
            file << "<maxErroneousBitsInBorderRate>" << params.maxErroneousBitsInBorderRate << "</maxErroneousBitsInBorderRate>" << endl;
            file << "<maxMarkerPerimeterRate>" << params.maxMarkerPerimeterRate << "</maxMarkerPerimeterRate>" << endl;
            file << "<minCornerDistanceRate>" << params.minCornerDistanceRate << "</minCornerDistanceRate>" << endl;
            file << "<minDistanceToBorder>" << params.minDistanceToBorder << "</minDistanceToBorder>" << endl;
            file << "<minMarkerDistanceRate>" << params.minMarkerDistanceRate << "</minMarkerDistanceRate>" << endl;
            file << "<minMarkerPerimeterRate>" << params.minMarkerPerimeterRate << "</minMarkerPerimeterRate>" << endl;
            file << "<minOtsuStdDev>" << params.minOtsuStdDev << "</minOtsuStdDev>" << endl;
            file << "<perspectiveRemoveIgnoredMarginPerCell>" << params.perspectiveRemoveIgnoredMarginPerCell << "</perspectiveRemoveIgnoredMarginPerCell>" << endl;
            file << "<perspectiveRemovePixelPerCell>" << params.perspectiveRemovePixelPerCell << "</perspectiveRemovePixelPerCell>" << endl;
            file << "<polygonalApproxAccuracyRate>" << params.polygonalApproxAccuracyRate << "</polygonalApproxAccuracyRate>" << endl;
            file << "<doCornerRefinement>" << (params.doCornerRefinement ? "true" : "false") << "</doCornerRefinement>" << endl;
            file << "</DetectorParameters>" << endl;
            file.close();
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
