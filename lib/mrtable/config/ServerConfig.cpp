#ifndef __SERVERCONFIG_CPP__
#define __SERVERCONFIG_CPP__

#include "DetectBounds.h"
#include "ServerConfig.h"
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/aruco.hpp>
#include <string>
#include "ContourParams.hpp"
#include "parsers.hpp"

using namespace mrtable::config;
using namespace std;

namespace mrtable {
    namespace config {
        cv::Ptr<mrtable::config::ContourParams> ServerConfig::contourParameters = ContourParams::create();
        cv::Ptr<aruco::DetectorParameters> ServerConfig::detectorParameters = cv::aruco::DetectorParameters::create();
        cv::Ptr<aruco::Dictionary> ServerConfig::dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_100);
        Mat ServerConfig::cameraMatrix = Mat1f(3, 3);
        Mat ServerConfig::distortionCoefficients = Mat1f(4, 1);
        int ServerConfig::dictionaryId = 1;
        int ServerConfig::cameraWidth = 1024;
        int ServerConfig::cameraHeight = 768;
        double ServerConfig::markerLength = 0.1;
        int ServerConfig::skippableFrames = 3;
        float ServerConfig::movementThreshold = 0.003;
        float ServerConfig::angleThreshold = 0.17;
        string ServerConfig::host = "127.0.0.1";
        int ServerConfig::cmd_port = 3332;
        int ServerConfig::udp_port = 3333;
        short int ServerConfig::tcp_port = 3333;
        int ServerConfig::web_port = 8080;
        bool ServerConfig::enable_udp = true;
        bool ServerConfig::enable_tcp = false;
        bool ServerConfig::enable_web = false;
        string ServerConfig::cameraParametersFile = "cameraParams.xml";
        string ServerConfig::contourParametersFile = "contourParams.xml";
        string ServerConfig::detectorParametersFile = "detectorParams.xml";
        string ServerConfig::boundsParametersFile = "boundsParams.xml";
        bool ServerConfig::udp_port_default = true;
        bool ServerConfig::tcp_port_default = true;
        bool ServerConfig::web_port_default = true; 


        bool ServerConfig::read(string filename){
            DetectBounds::setDefaults();
            if (filename.empty()) {
                return false;
            }
            FileStorage fs(filename, FileStorage::READ);
            if (!fs.isOpened())
                return false;

            string temp;

            fs["detectorParameters"] >> temp; 
            if (temp.compare("default") != 0) {
                detectorParametersFile = temp; 
            }
            if (!readDetectorParameters(detectorParametersFile, detectorParameters)) {
                std::cerr << "Could not open " << detectorParametersFile << " - using default Aruco Detector Parameters" << std::endl;
            }


            fs["cameraWidth"] >> temp;
            if (!temp.empty() && temp.compare("default") != 0) {
                cameraWidth = stoi(temp);
            }

            fs["cameraHeight"] >> temp;
            if (!temp.empty() && temp.compare("default") != 0) {
                cameraHeight = stoi(temp);
            }

            fs["boundsParameters"] >> temp;
            if (temp.compare("default") != 0) {
                boundsParametersFile = temp;
            }
            if (!DetectBounds::read(boundsParametersFile)) {
                std::cerr << "Could not open " << boundsParametersFile << " - using default bounds parameters" << std::endl;
            }

            fs["markerLength"] >> temp;
            if (!temp.empty() && temp.compare("default") != 0) {
                markerLength = stof(temp);
            }

            fs["skippableFrames"] >> temp;
            if (!temp.empty() && temp.compare("default") != 0) {
                skippableFrames = stoi(temp);
            }

            fs["movementThreshold"] >> temp;
            if (!temp.empty() && temp.compare("default") != 0) {
                movementThreshold = stof(temp);
            }

            fs["angleThreshold"] >> temp;
            if (!temp.empty() && temp.compare("default") != 0) {
                angleThreshold = stof(temp);
            }

            fs["dictionaryId"] >> temp;
            if (!temp.empty() && temp.compare("default") != 0) {
                dictionaryId = stoi(temp);
            }
            dictionary.release();
            dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));

            fs["contourParameters"] >> temp;
            if (temp.compare("default") != 0) {
                contourParametersFile = temp;
            }
            contourParameters.release();
            contourParameters = mrtable::config::ContourParams::create(contourParametersFile);

            fs["cameraParameters"] >> temp;
            if (temp.compare("default") != 0 ) {
                cameraParametersFile = temp;
            }
            if (!mrtable::config::readCameraParameters(cameraParametersFile, cameraMatrix, distortionCoefficients)) {
                std::cerr << "Could not open " << cameraParametersFile << " - marker detection may be inaccurate!" << std::endl;
            }

            fs["host"] >> temp;
            if (!temp.empty()) {
                if (temp.compare("default") != 0) {
                    host = temp;
                }
            }

            fs["cmdPort"] >> temp;
            if (!temp.empty() && temp.compare("default") != 0) {
                cmd_port = (short int)stoi(temp);
            }

            fs["udpPort"] >> temp;
            if (!temp.empty()) {
                enable_udp = true;
                if (temp.compare("default") != 0) {
                    udp_port = stoi(temp);
                }
            }

            fs["tcpPort"] >> temp;
            if (!temp.empty()) {
                enable_tcp = true;
                if (temp.compare("default") != 0) {
                    tcp_port = stoi(temp);
                }
            }

            fs["webPort"] >> temp;
            if (!temp.empty()) {
                enable_web = true;
                if (temp.compare("default") != 0) {
                    web_port = stoi(temp);
                }
            }

            return true;
        }

        bool ServerConfig::write(string filename) {
            FileStorage fs(filename, FileStorage::WRITE);
            if (!fs.isOpened())
                return false;

            fs << "detectorParameters" << detectorParametersFile;
            fs << "contourParameters" << contourParametersFile;
            fs << "cameraParameters" << cameraParametersFile;
            fs << "boundsParameters" << boundsParametersFile;
            fs << "dictionaryId" << dictionaryId;
            fs << "markerLength" << markerLength;
            fs << "skippableFrames" << skippableFrames;
            fs << "movementThreshold" << movementThreshold;
            fs << "angleThreshold" << angleThreshold;
            fs << "host" << host.c_str();

            if (enable_udp) {
                fs << "udpPort" << (udp_port_default ? to_string(udp_port) : "default");
            }
            if (enable_tcp) {
                fs << "tcpPort" << (tcp_port_default ? to_string(tcp_port) : "default");
            }
            if (enable_web) {
                fs << "webPort" << (web_port_default ? to_string(web_port) : "default");
            }
            fs.release();
            if (!writeDetectorParameters(detectorParametersFile, detectorParameters)) {
                std::cerr << "Could not write detector parameters to " << detectorParametersFile << std::endl;
            }
            if (!writeCameraParameters(cameraParametersFile, cameraMatrix, distortionCoefficients)) {
                std::cerr << "Could not write camera matrix and distortion coefficients to " << cameraParametersFile << std::endl;
            }
            if (!contourParameters->write(contourParametersFile)) {
                std::cerr << "Could not write contour parameters to " << contourParametersFile << std::endl;
            }
            if (!DetectBounds::write(boundsParametersFile)) {
                std::cerr << "Could not write bounds parameters to " << boundsParametersFile << std::endl;
            }
            return true;
        }

        void ServerConfig::dump(basic_ostream<char>& outs) {
            outs << "*** Contour Parameters ***" << std::endl << contourParameters << std::endl;
            outs << "*** Detector Parameters ***" << std::endl << detectorParameters << std::endl;
            outs << "*** Detector Dictionary ***" << std::endl;
            switch (dictionaryId) {
                case 0: outs << "DICT_4X4_50" << std::endl; break;
                case 1: outs << "DICT_4X4_100" << std::endl; break;
                case 2: outs << "DICT_4X4_250" << std::endl; break;
                case 3: outs << "DICT_4X4_1000" << std::endl; break;
                case 4: outs << "DICT_5X5_50" << std::endl; break;
                case 5: outs << "DICT_5X5_100" << std::endl; break;
                case 6: outs << "DICT_5X5_250" << std::endl; break;
                case 7: outs << "DICT_5X5_1000" << std::endl; break;
                case 8: outs << "DICT_6X6_50" << std::endl; break;
                case 9: outs << "DICT_6X6_100" << std::endl; break;
                case 10: outs << "DICT_6X6_250" << std::endl; break;
                case 11: outs << "DICT_6X6_1000" << std::endl; break;
                case 12: outs << "DICT_7X7_50" << std::endl; break;
                case 13: outs << "DICT_7X7_100" << std::endl; break;
                case 14: outs << "DICT_7X7_250" << std::endl; break;
                case 15: outs << "DICT_7X7_1000" << std::endl; break;
                case 16: outs << "DICT_ARUCO_ORIGINAL" << std::endl; break;
                default: outs << "Unknown Dictionary" << std::endl; break;
            }
            DetectBounds::dump(outs);
            outs << "*** Aruco Marker Thresholds ***" << std::endl;
            outs << "skippableFrames: " << skippableFrames << std::endl;
            outs << "movementThreshold: " << movementThreshold << std::endl;
            outs << "angleThreshold: " << angleThreshold << std::endl;
            outs << "*** Camera Matrix ***" << std::endl << cameraMatrix << std::endl;
            outs << "*** Distortion Coefficients ***" << std::endl << distortionCoefficients << std::endl;
            outs << "*** Server Parameters ***" << std::endl;
            outs << "Host: " << host << std::endl;
            if (enable_udp) {
                outs << "UDP Port: " << udp_port << (udp_port_default ? " (default)" : "") << std::endl;
            }
            if (enable_tcp) {
                outs << "TCP Port: " << tcp_port << (tcp_port_default ? " (default)" : "") << std::endl;
            }
            if (enable_web) {
                outs << "Web Port: " << web_port << (web_port_default ? " (default)" : "") << std::endl;
            }
            outs << std::endl;
        }

        ServerConfig::ServerConfig() {
        }


        ServerConfig::~ServerConfig() {
            contourParameters.release();
            distortionCoefficients.release();
            dictionary.release();
        }

    }
}

#endif
