#include <mrtable/mrtable.hpp>
#include <iostream>
#include <chrono>
#include <string>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <thread>
#include <tuio/TuioServer.h>
#include <boost/asio.hpp>
#include <thread>

using namespace mrtable::server;
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

string configFile = "serverConfig.xml";
bool verbose = false;

cv::Ptr<FrameProcessor> process;
TableServer* server = NULL;
Ptr< MutexQueue<string> > msgQueue;
Ptr< MutexQueue<string> > sendQueue;

int main(int argc, char** argv) {
    CommandLineParser parser(argc, argv, keys);
    parser.about(about);

    if (parser.has("h")) {
        parser.printMessage();
        return 0;
    }

    // Create msgQueue for communication between CommandServer and TableServer
    msgQueue = MutexQueue< string >::create();
    sendQueue = MutexQueue< string >::create();

    // Load configuration file
    if (parser.has("c")) {
        ServerConfig::read(parser.get<String>("c"));
    } else {
        ServerConfig::read("serverConfig.xml");
    }

    // Create CommandServer thread
    boost::asio::io_service ioservice;
    CommandServer cmdServer(ioservice, msgQueue, sendQueue, ServerConfig::cmd_port);
    std::thread thread1{[&ioservice]() { ioservice.run(); }};

    // Create TableServer (run in main thread)
    server = new TableServer(msgQueue, sendQueue);

    verbose = parser.has("verbose");

    if (verbose) {
        ServerConfig::dump(std::cout);
    }

    if (parser.has("o")) {
        ServerConfig::write("serverConfig.xml");
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
    server->start();

    result_t aggregate;
    aggregate.frames = 0;
    aggregate.elapsed = 0;

    thread1.join();
}
