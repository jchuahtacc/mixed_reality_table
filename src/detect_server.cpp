#include <mrtable/mrtable.hpp>
#include <iostream>
#include <chrono>
#include <string>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <thread>
#include <tuio/TuioServer.h>

using namespace mrtable::config;
using namespace mrtable::process;
using namespace mrtable::sources;
using namespace mrtable::data;
using namespace TUIO;

namespace {
const char* about = "Basic marker detection";
const char* keys  =
        "{h        |       | Print this help message}"
        "{v        |       | Input from video file (to play on a loop), if ommited, input comes from camera }"
        "{ff       |       | Fast-forward - when previewing video, ignore source FPS}"
        "{c        |       | Server configuration XML file (otherwise, defaults will be used)}"
        "{o        |       | Output configuration XML (serverConfig.xml, cameraParameters.xml, etc..)}"
        "{ci       | 0     | Camera id if input doesnt come from video (-v) }"
        "{p        |       | Show preview in GUI window }"
        "{verbose  |       | Verbose output of settings }"
        "{r        |       | Show rejected candidates during preview }";
}

ServerConfig* config = NULL;
string configFile = "serverConfig.xml";
bool verbose = false;

cv::Ptr<FrameProcessor> process;
DetectServer* server = NULL;
Mat image;

int main(int argc, char** argv) {
    CommandLineParser parser(argc, argv, keys);
    parser.about(about);

    if (parser.has("h")) {
        parser.printMessage();
        return 0;
    }

    config = new ServerConfig(parser.get<String>("c"));
    server = new DetectServer(config);

    verbose = parser.has("verbose");

    if (verbose) {
        cout << config;
    }

    if (parser.has("o")) {
        config->write("serverConfig.xml");
    }

    cv::Ptr< mrtable::sources::VideoSource > vidSource;

    if (parser.has("v")) {
        string vidFile = parser.get<String>("v");
        if (vidFile.empty()) {
            std::cerr << "No video file specified in -v flag. (Try -v=file.mpg)" << std::endl;
            return -1;
        }
        vidSource = mrtable::sources::VideoSource::create(vidFile);
    } else {
        vidSource = mrtable::sources::VideoSource::create(parser.get<int>("ci"));
    }

    std::cout << "Processing video source: " << *vidSource << std::endl;
    server->setVideoSource(vidSource);
    server->setPreview(parser.has("p"));

    if (parser.has("v")) {
        std::cout << "Processing video file " << vidSource->getSource() << std::endl;
        int result = 0;
        while (result != 27) {
            std::cout << "Press ESC to exit, or any other key to replay video..." << std::endl;
            result = server->start();
            vidSource->reset();
        }
        SharedData::destroy();
        return 0;
    }


    result_t aggregate;
    aggregate.frames = 0;
    aggregate.elapsed = 0;

    SharedData::destroy();
}
