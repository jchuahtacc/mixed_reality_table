#include <tuio/TuioServer.h>
#include <mrtable/mrtable.hpp>
#include <iostream>
#include <chrono>
#include <string>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/aruco.hpp>

using namespace mrtable::config;
using namespace mrtable::process;
using namespace mrtable::sources;

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

cv::Ptr<ServerConfig> config;
string configFile = "serverConfig.xml";
bool verbose = false;

cv::Ptr<FrameProcessor> process;
cv::Ptr<ProcessorOutput> outputs;
VideoSource* vidSource = NULL;
ProcessQueue* proc;
Mat image;

int processVideo() {
    if (vidSource == NULL) {
        std::cerr << "No video source specified!" << std::endl;
        return 27;
    }
    result_t aggregate;
    aggregate.frames = 0;
    aggregate.elapsed = 0;
    while (vidSource->getFrame(image)) {
        // std::cout << "processVideo(): " << image.cols << "x" << image.rows << std::endl;
        result_t result = proc->process(image);
        if (outputs->get<int>(RESULT_KEY_DISPLAYFRAME_KEYPRESS) == 27) {
            return 27;
        }
        aggregate.frames++;
        aggregate.elapsed += result.elapsed;
    }
    std::cout << aggregate << std::endl;
    return waitKey(0);
}

int main(int argc, char** argv) {
    CommandLineParser parser(argc, argv, keys);
    parser.about(about);

    if (parser.has("h")) {
        parser.printMessage();
        return 0;
    }

    config = ServerConfig::create(parser.get<String>("c"));

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

    std::cout << *vidSource << std::endl;

    // Prepare frame processors
    outputs = mrtable::process::ProcessorOutput::create();
    proc = new ProcessQueue(config, outputs);

    //proc->addProcessor(mrtable::process::Grayscale::create());
    //proc->addProcessor(mrtable::process::Otsu::create());
    //proc->addProcessor(mrtable::process::OtsuCalc::create());
    //proc->addProcessor(mrtable::process::Canny::create());
    
    if (parser.has("p")) { 
        int waitTime = parser.has("ff") ? 1 :  vidSource->waitTime;
        proc->addProcessor(mrtable::process::DisplayFrame::create(waitTime));
    }

    if(parser.has("v")) {
        std::cout << "Playing video file " << vidSource->getSource() << std::endl;
        int result = 0;
        while (result != 27) {
            std::cout << "Press ESC to exit, or any other key to replay video..." << std::endl;
            result = processVideo();
            vidSource->reset();
        }
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
}
