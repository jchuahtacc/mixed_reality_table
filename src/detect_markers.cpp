#include <opencv2/highgui/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <iostream>
#include <mrtable/settings.hpp>

using namespace std;
using namespace cv;
using namespace mrtable::settings;
using namespace std::chrono;

namespace {
const char* about = "Basic marker detection";
const char* keys  =
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

typedef struct result_t {
    int frames;
    int detected;
    int elapsed;
} result_t;

int dictionaryId;
bool showRejected;
bool estimatePose;
bool preview;
bool output;
bool verbose;
float markerLength;

aruco::DetectorParameters detectorParams;
aruco::Dictionary dictionary;
Mat camMatrix, distCoeffs;
vector< int > ids;
vector< vector< Point2f > > corners, rejected;
vector< Vec3d > rvecs, tvecs;

void outputResults(result_t result) {
    cout << "Frames processed: " << result.frames << endl;
    cout << "Milliseconds elapsed: " << result.elapsed << endl;
    cout << "Markers detected: " << result.detected << endl;
    cout << "Average frames per second: " << (result.frames * 1000.0 / result.elapsed) << endl;
    cout << "Average markers detected per frame: " << (result.detected * 1.0 / result.frames) << endl;
}

string getFilename(string filename) {
    for (int i = filename.length(); i >= 0; i--) {
        if (filename[i] == '/') {
            return filename.substr(i + 1);
        }
    }
    return filename;
}

result_t processImage(Mat image, Mat* imageCopy) {
    steady_clock::time_point detectBegin = steady_clock::now();

    // detect markers and estimate pose
    aruco::detectMarkers(image, dictionary, corners, ids, detectorParams, rejected);
    if(estimatePose && ids.size() > 0)
        aruco::estimatePoseSingleMarkers(corners, markerLength, camMatrix, distCoeffs, rvecs,
                                         tvecs);
    steady_clock::time_point detectEnd = steady_clock::now();
    long elapsed = duration_cast<milliseconds>(detectEnd - detectBegin).count();
 
    // draw results
    if (imageCopy) {
        image.copyTo(*imageCopy);
        if(ids.size() > 0) {
            aruco::drawDetectedMarkers(*imageCopy, corners, ids);

            if(estimatePose) {
                for(unsigned int i = 0; i < ids.size(); i++)
                    aruco::drawAxis(*imageCopy, camMatrix, distCoeffs, rvecs[i], tvecs[i],
                                    markerLength * 0.5f);
            }
        }

        if(showRejected && rejected.size() > 0)
            aruco::drawDetectedMarkers(*imageCopy, rejected, noArray(), Scalar(100, 0, 255));
    }

    result_t result;
    result.frames = 1;
    result.elapsed = elapsed;
    result.detected = ids.size();

    return result;
}

/**
 */
int main(int argc, char *argv[]) {
    CommandLineParser parser(argc, argv, keys);
    parser.about(about);

    if(argc < 2) {
        parser.printMessage();
        return 0;
    }

    dictionaryId = parser.get<int>("d");
    showRejected = parser.has("r");
    estimatePose = parser.has("c");
    markerLength = parser.get<float>("l");
    preview = parser.has("p");
    output = parser.has("o");
    verbose = parser.has("verbose");
    String overrideParameters = parser.get<String>("op");

    if(parser.has("dp")) {
        bool readOk = readDetectorParameters(parser.get<string>("dp"), &detectorParams);
        if(!readOk) {
            cerr << "Invalid detector parameters file" << endl;
            return 0;
        }
    }
    detectorParams.doCornerRefinement = true; // do corner refinement in markers

    int camId = parser.get<int>("ci");

    if(!parser.check()) {
        parser.printErrors();
        return 0;
    }

    dictionary = getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));

    if(estimatePose) {
        bool readOk = readCameraParameters(parser.get<string>("c"), camMatrix, distCoeffs);
        if(!readOk) {
            cerr << "Invalid camera file" << endl;
            return 0;
        }
    }

    if (!overrideParameters.empty()) {
        parseDetectorParameters(overrideParameters.c_str(), &detectorParams);
    }

    if (verbose) {
        cout << "Camera Matrix: " << endl << camMatrix << endl;
        cout << "Distortion Coefficients: " << endl << distCoeffs << endl;
        printDetectorParameters(detectorParams);
    }

    if(output) {
        writeCameraParameters("output_cameraParams.xml", camMatrix, distCoeffs);
        writeDetectorParameters("output_detectorParams.xml", detectorParams);
    }

    if (parser.has("i")) {
        string filename = parser.get<String>("i");
        Mat image, imageCopy;
        image = imread(filename);
        result_t result = processImage(image, &imageCopy);
        if (output) {
            imwrite("output_" + getFilename(filename), imageCopy); 
        }
        if (preview) {
            cout << "Press the Escape key to continue..." << endl;
            imshow("out", imageCopy);
            char key = (char)waitKey(0);
        }
        outputResults(result);
        return 0;
    }

    String video;
    if(parser.has("v")) {
        video = parser.get<String>("v");
    }

    VideoCapture inputVideo;
    int waitTime;
    bool videoFile = false;
    if(!video.empty()) {
        videoFile = true;
        inputVideo.open(video);
        waitTime = 0;
    } else {
        inputVideo.open(camId);
        waitTime = 10;
    }

    VideoWriter outputVideo;

    // Disable camera capture -- can't get it working
    if (!videoFile && output) {
        output = false;
    }
    if (output) {
        int ex;
        Size S = Size((int) inputVideo.get(CV_CAP_PROP_FRAME_WIDTH), (int) inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT));
 
        string outputFilename;
        if (!videoFile) {
            ex = CV_FOURCC('H','2','6','3');
            std::ostringstream oss;
            oss << "camera_" << camId << ".avi";
            outputFilename = "output_" + oss.str(); 
        } else {
            ex = static_cast<int>(inputVideo.get(CV_CAP_PROP_FOURCC));
            outputFilename = "output_" + getFilename(video);
        }
        outputVideo.open(outputFilename, ex, inputVideo.get(CV_CAP_PROP_FPS), S, true);
        if (!outputVideo.isOpened()) {
            cout << "Unable to open video file for output: " << ("output_" + getFilename(video)) << endl;
            return -1;
        }
    }

    result_t aggregate;

    if (preview) {
        cout << "Press the Escape key to continue..." << endl;
    }


    while(inputVideo.grab()) {
        Mat image, imageCopy;
        inputVideo.retrieve(image);
        
        result_t result;

        if (preview || output) {
            result = processImage(image, &imageCopy);
        } else {
            result = processImage(image, NULL);
        }

        aggregate.frames++;
        aggregate.detected += result.detected;
        aggregate.elapsed += result.elapsed;

        if (output) {
            outputVideo << imageCopy;
        }

        if (preview) {
            imshow("out", imageCopy);
            char key = (char)waitKey(waitTime);
            if(key == 27) break;
        }
    }

    outputResults(aggregate);

    return 0;
}
