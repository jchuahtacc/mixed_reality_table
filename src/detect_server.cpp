#include <tuio/TuioServer.h>
#include <mrtable/MRTable.hpp>
#include <iostream>
#include <chrono>
#include <string>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/aruco.hpp>

using namespace mrtable::config;

namespace {
const char* about = "Basic marker detection";
const char* keys  =
        "{h        |       | Print this help message}"
        "{v        |       | Input from video file (to play on a loop), if ommited, input comes from camera }"
        "{c        |       | Server configuration XML file }"
        "{o        |       | Output configuration XML files }"
        "{ci       | 0     | Camera id if input doesnt come from video (-v) }"
        "{p        |       | Show preview in GUI window }"
        "{verbose  |       | Verbose output of settings }"
        "{r        |       | Show rejected candidates during preview }";
}

cv::Ptr<ServerConfig> config;
string configFile = "serverConfig.xml";

int main(int argc, char** argv) {
    CommandLineParser parser(argc, argv, keys);
    parser.about(about);

    if (parser.has("h")) {
        parser.printMessage();
        return 0;
    }

    config = ServerConfig::create(parser.get<String>("c"));

    if (parser.has("o")) {
        config->write("serverConfig.xml");
    }
    


}
