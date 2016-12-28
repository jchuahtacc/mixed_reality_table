#ifndef __SERVERCONFIG_HPP__
#define __SERVERCONFIG_HPP__

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
        class ServerConfig {
            public: 
                cv::Ptr<mrtable::config::ContourParams> contourParameters;
                cv::Ptr<aruco::DetectorParameters> detectorParameters;
                cv::Ptr<aruco::Dictionary> dictionary;
                Mat cameraMatrix, distortionCoefficients;
                int dictionaryId = 1;
                double markerLength = 0.1;
                int skippableFrames = 3;
                int movementThreshold = 5;
                float angleThreshold = 0.17;
                string host = "127.0.0.1";
                int udp_port = 3333;
                int tcp_port = 3333;
                int web_port = 8080;
                bool enable_udp = true;
                bool enable_tcp = false;
                bool enable_web = false;

                static cv::Ptr<ServerConfig> create(string filename = "") {
                    return cv::makePtr<ServerConfig>(filename);
                }

                bool read(string filename) {
                    ServerConfig* params = this;
                    FileStorage fs(filename, FileStorage::READ);
                    if (!fs.isOpened())
                        return false;

                    string temp;

                    fs["detectorParameters"] >> temp; 
                    if (temp.compare("default") != 0) {
                        detectorParametersFile = temp; 
                    }
                    detectorParameters.release();
                    if (!readDetectorParameters(detectorParametersFile, detectorParameters)) {
                        std::cerr << "Could not open " << detectorParametersFile << " - using default Aruco Detector Parameters" << std::endl;
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
                        movementThreshold = stoi(temp);
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
                        std::cerr << "Could not open " << cameraParametersFile << " - pose detection may be inaccurate!" << std::endl;
                    }

                    fs["host"] >> temp;
                    if (!temp.empty()) {
                        if (temp.compare("default") != 0) {
                            host = temp;
                        }
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

                bool write(string filename) {
                    ServerConfig* params = this;
                    FileStorage fs(filename, FileStorage::WRITE);
                    if (!fs.isOpened())
                        return false;

                    fs << "detectorParameters" << detectorParametersFile;
                    fs << "contourParameters" << contourParametersFile;
                    fs << "cameraParameters" << cameraParametersFile;
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
                    return true;
                }

                friend basic_ostream<char>& operator<<(basic_ostream<char>& outs, ServerConfig params) {
                    outs << "*** Contour Parameters ***" << std::endl << params.contourParameters << std::endl;
                    outs << "*** Detector Parameters ***" << std::endl << params.detectorParameters << std::endl;
                    outs << "*** Detector Dictionary ***" << std::endl;
                    switch (params.dictionaryId) {
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
                    outs << "*** Aruco Marker Thresholds ***" << std::endl;
                    outs << "skippableFrames: " << params.skippableFrames << std::endl;
                    outs << "movementThreshold: " << params.movementThreshold << std::endl;
                    outs << "angleThreshold: " << params.angleThreshold << std::endl;
                    outs << "*** Camera Matrix ***" << std::endl << params.cameraMatrix << std::endl;
                    outs << "*** Distortion Coefficients ***" << std::endl << params.distortionCoefficients << std::endl;
                    outs << "*** Server Parameters ***" << std::endl;
                    outs << "Host: " << params.host << std::endl;
                    if (params.enable_udp) {
                        outs << "UDP Port: " << params.udp_port << (params.udp_port_default ? " (default)" : "") << std::endl;
                    }
                    if (params.enable_tcp) {
                        outs << "TCP Port: " << params.tcp_port << (params.tcp_port_default ? " (default)" : "") << std::endl;
                    }
                    if (params.enable_web) {
                        outs << "Web Port: " << params.web_port << (params.web_port_default ? " (default)" : "") << std::endl;
                    }
                    outs << std::endl;

                    return outs; 
                }

                friend basic_ostream<char>& operator<<(basic_ostream<char>& outs, cv::Ptr<ServerConfig> params) {
                    outs << *(params.get());
                    return outs;
                }

                ServerConfig(string filename = "") {
                    contourParameters = ContourParams::create();
                    detectorParameters = cv::aruco::DetectorParameters::create();
                    dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_100);
                    
                    if (!filename.empty() && !read(filename)) {
                        std::cerr << "Unable to read server configuration from " << filename << std::endl;
                    }
                }


                ~ServerConfig() {
                    contourParameters.release();
                    detectorParameters.release();
                    dictionary.release();
                }

            private:
                string cameraParametersFile = "cameraParameters.xml";
                string contourParametersFile = "contourParameters.xml";
                string detectorParametersFile = "detectorParameters.xml";
                bool udp_port_default = true;
                bool tcp_port_default = true;
                bool web_port_default = true; 
        };
    }
}

#endif
