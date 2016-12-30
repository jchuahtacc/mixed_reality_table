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
VideoSource* vidSource = NULL;
ProcessQueue* proc = NULL;
TuioServer* server = NULL;

Mat image;

int processVideo(bool headless) {
    if (vidSource == NULL) {
        std::cerr << "No video source specified!" << std::endl;
        return 27;
    }
    result_t aggregate;
    aggregate.frames = 0;
    aggregate.elapsed = 0;
    while (vidSource->getFrame(image)) {
        result_t result = proc->process(image);
        if (!headless) {
            if (SharedData::get<int>(RESULT_KEY_DISPLAYFRAME_KEYPRESS) == 27) {
                return 27;
            }
        }
        aggregate.frames++;
        aggregate.elapsed += result.elapsed;
        aggregate.detected += result.detected;
    }
    std::cout << aggregate << std::endl;
    if (headless) return 27;
    return waitKey(0);
}

int main(int argc, char** argv) {
    CommandLineParser parser(argc, argv, keys);
    parser.about(about);

    if (parser.has("h")) {
        parser.printMessage();
        return 0;
    }

    config = new ServerConfig(parser.get<String>("c"));

    SharedData::put(KEY_CONFIG, config);

    verbose = parser.has("verbose");

    if (verbose) {
        cout << config;
    }

    if (parser.has("o")) {
        config->write("serverConfig.xml");
    }

    if (parser.has("v")) {
        string vidFile = parser.get<String>("v");
        if (vidFile.empty()) {
            std::cerr << "No video file specified in -v flag. (Try -v=file.mpg)" << std::endl;
            return -1;
        }
        vidSource = new VideoSource(vidFile);
    } else {
        vidSource = new VideoSource(parser.get<int>("ci")); 
    }

    OscSender* sender;
    //server = new TuioServer(config->host.c_str(), config->udp_port);
    if (config->enable_udp) {
        sender = new UdpSender(config->host.c_str(), config->udp_port);
        server = new TuioServer(sender);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    if (config->enable_tcp) {
        sender = new TcpSender(config->tcp_port);
        if (server == NULL) {
            server = new TuioServer(sender);
        } else {
            server->addOscSender(sender);
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    if (config->enable_web) {
        sender = new WebSockSender(config->web_port);
        if (server == NULL) {
            server = new TuioServer(sender);
        } else {
            server->addOscSender(sender);
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    if (server == NULL) {
        std::cerr << "Warning: No TUIO server was specified in configuration! Attempting to start default server." << std::endl;
        sender = new UdpSender("127.0.0.1", 3333);
        server = new TuioServer(sender);
    }

    std::cout << *vidSource << std::endl;

    // Prepare frame processors
    SharedData::put(KEY_TUIO_SERVER, server);

    proc = new ProcessQueue();
    proc->addProcessor(mrtable::process::Grayscale::create());
    proc->addProcessor(mrtable::process::Otsu::create());
    //proc->addProcessor(mrtable::process::OtsuCalc::create());
    //proc->addProcessor(mrtable::process::Canny::create());
    proc->addProcessor(mrtable::process::Aruco::create());
    proc->addProcessor(mrtable::process::ArucoCompute::create());
    proc->addProcessor(mrtable::process::Contour::create());
    
    if (parser.has("p")) { 
        int waitTime = parser.has("ff") ? 1 :  vidSource->waitTime;
        proc->addProcessor(mrtable::process::DisplayFrame::create(waitTime));
    }

    if (parser.has("v")) {
        std::cout << "Processing video file " << vidSource->getSource() << std::endl;
        int result = 0;
        while (result != 27) {
            std::cout << "Press ESC to exit, or any other key to replay video..." << std::endl;
            result = processVideo(!parser.has("p"));
            vidSource->reset();
        }
        SharedData::destroy();
        return 0;
    }


    result_t aggregate;
    aggregate.frames = 0;
    aggregate.elapsed = 0;

    while (vidSource->getFrame(image)) {
        result_t result = proc->process(image);
        aggregate.frames++;
        aggregate.elapsed += result.elapsed;
    }
    SharedData::destroy();
}
