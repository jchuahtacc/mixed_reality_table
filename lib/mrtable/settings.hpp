
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
       void parseCameraSettings(const char * filename, Mat *cameraMatrix, Mat *distCoeffs) throw() {
            FileStorage fs(filename, FileStorage::READ);
            Mat cameraMatrix2, distCoeffs2;
            fs["cameraMatrix"] >> cameraMatrix2;
            fs["distCoeffs"] >> distCoeffs2;
            cameraMatrix2.copyTo(*cameraMatrix);
            distCoeffs2.copyTo(*distCoeffs);
            fs.release();
        }

        double getVal(xml_node<char> * node, const char * child) throw() {
            xml_node<char> *child_node = node->first_node(child);
            if (!child_node) {
                throw (std::runtime_error("Node not found"));
            }
            string contents = string(child_node->value());
            return stod(contents);
        }

        void parseDetectorSettings(const char * filename, DetectorParameters* params) throw() {
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
    }
}

#endif
