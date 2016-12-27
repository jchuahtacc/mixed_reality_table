#include <tuio/TuioServer.h>
#include <mrtable/parsers.hpp>
#include <iostream>
#include <chrono>
#include <string>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/aruco.hpp>

namespace {
const char* about = "Basic marker detection";
const char* keys  =
        "{h        |       | Print this help message}"
        "{d        |       | dictionary: DICT_4X4_50=0, DICT_4X4_100=1, DICT_4X4_250=2,"
        "DICT_4X4_1000=3, DICT_5X5_50=4, DICT_5X5_100=5, DICT_5X5_250=6, DICT_5X5_1000=7, "
        "DICT_6X6_50=8, DICT_6X6_100=9, DICT_6X6_250=10, DICT_6X6_1000=11, DICT_7X7_50=12,"
        "DICT_7X7_100=13, DICT_7X7_250=14, DICT_7X7_1000=15, DICT_ARUCO_ORIGINAL = 16}"
        "{v        |       | Input from video file, if ommited, input comes from camera }"
        "{i        |       | Input from a single image file }"
        "{ci       | 0     | Camera id if input doesnt come from video (-v) }"
        "{c        |       | Camera intrinsic parameters. Needed for camera pose }"
        "{l        | 0.1   | Marker side lenght (in meters). Needed for correct scale in camera pose }"
        "{dp       |       | File of marker detector parameters }"
        "{op       |       | List of overridden detector parameters, i.e. adaptiveThreshConstant=5,doCornerRefinement=false}"
        "{o        |       | Output camera and detector parameters and preview renderings. File prefix is output_}"
        "{p        |       | Show preview in GUI window }"
        "{verbose  |       | Verbose output of settings }"
        "{r        |       | show rejected candidates too }";
}
int main(int argc, char** argv) {
    CommandLineParser parser(argc, argv, keys);
    parser.about(about);

    if(parser.has("h")) {
        parser.printMessage();
        return 0;
    }

}
