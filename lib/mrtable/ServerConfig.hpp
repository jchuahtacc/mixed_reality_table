#ifndef __SERVERCONFIG_HPP__
#define __SERVERCONFIG_HPP__

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/aruco.hpp>
#include <mrtable/ContourParams.hpp> 
#include <string>

using namespace mrtable::config;

namespace mrtable {
    namespace config {
        class ServerConfig {
            public: 
                cv::Ptr<mrtable::config::ContourParams> contourParameters;
                cv::Ptr<aruco::DetectorParameters> detectorParameters;
                cv::Ptr<aruco::Dictionary> dictionary;
                Mat cameraMatrix, distortionCoefficients;

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
                    if (!readDetectorParameters(detectorParametersFile, detectorParameters)) {
                        std::cerr << "Could not open " << detectorParametersFile << " - using default Aruco Detector Parameters" << std::endl;
                        delete detectorParameters;
                        detectorParameters = cv::aruco::DetectorParameters::create();
                    }

                    fs["dictionaryId"] >> temp;
                    dictionaryId = stoi(temp);
                    delete dictionary;
                    dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));

                    fs["contourParameters"] >> temp;
                    if (temp.compare("default") != 0) {
                        contourParametersFile = temp;
                    }
                    delete contourParameters;
                    contourParameters = mrtable::config::ContourParams::create(contourParametersFile);

                    fs["cameraParameters"] >> temp;
                    if (temp.compare("default") != 0 ) {
                        cameraParametersFile = temp;
                    }
                    if (!mrtable::config::readCameraParameters(cameraParametersFile, cameraMatrix, distortionCoefficients)) {
                        std::cerr << "Could not open " << cameraParametersFile << " - pose detection may be inaccurate!" << std::endl;
                    }

                    return true;
                }

                bool write(string filename) {
                    ServerConfig* params = this;
                    FileStorage fs(filename, FileStorage::WRITE);
                    if (!fs.isOpened())
                        return false;
                    fs.release();
                    return true;
                }

                friend basic_ostream<char>& operator<<(basic_ostream<char>& outs, ServerConfig params) {

                    /*
                    outs << "minPointRatio" << params.minPointRatio << endl;
                    outs << "maxPointRatio" << params.maxPointRatio << endl;
                    outs << "minRadiusRatio" << params.minRadiusRatio << endl;
                    outs << "maxRadiusRatio" << params.maxRadiusRatio << endl;
                    */
                    return outs; 
                }

                friend basic_ostream<char>& operator<<(basic_ostream<char>& outs, cv::Ptr<ServerConfig> params) {
                    outs << *(params.get());
                    return outs;
                }

                ServerConfig() {
                    contourParameters = ContourParams::create();
                    detectorParameters = cv::aruco::DetectorParameters::create();
                    dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_100);
                }

                ServerConfig(string filename="") {
                    read(filename);
                }

                ~ServerConfig() {
                    delete contourParameters;
                    delete detectorParameters;
                    delete dictionary;
                }

            private:
                string cameraParametersFile = "cameraParameters.xml";
                string contourParametersFile = "contourParameters.xml";
                string detectorParametersFile = "detectorParameters.xml";
                int dictionaryId;
        };
    }
}

#endif
