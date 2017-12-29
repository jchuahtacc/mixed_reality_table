#ifndef __SERVERCONFIG_H__
#define __SERVERCONFIG_H__

#include "DetectBounds.h"
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/aruco.hpp>
#include <string>
#include <memory>
#include "ContourParams.h"
#include "parsers.h"

using namespace mrtable::config;
using namespace std;

namespace mrtable {
    namespace config {
        class ServerConfig {
            public: 
                static shared_ptr<mrtable::config::ContourParams> contourParameters;
                static cv::Ptr<aruco::DetectorParameters> detectorParameters;
                static cv::Ptr<aruco::Dictionary> dictionary;
                static Mat cameraMatrix;
                static Mat distortionCoefficients;
                static int dictionaryId;
                static int cameraWidth;
                static int cameraHeight;
                static double markerLength;
                static int skippableFrames;
                static float movementThreshold;
                static float angleThreshold;
                static string host;
                static int cmd_port;
                static int udp_port;
                static short int tcp_port;
                static int web_port;
                static bool enable_udp;
                static bool enable_tcp;
                static bool enable_web;

                static bool read(string);

                static bool write(string);

                static void dump(basic_ostream<char>&);

                friend basic_ostream<char>& operator<<(basic_ostream<char>&, cv::Ptr<ServerConfig>);

                ~ServerConfig();

            private:

                ServerConfig();

                static string cameraParametersFile;
                static string contourParametersFile;
                static string detectorParametersFile;
                static string boundsParametersFile;
                static bool udp_port_default;
                static bool tcp_port_default;
                static bool web_port_default; 
        };
    }
}

#endif
