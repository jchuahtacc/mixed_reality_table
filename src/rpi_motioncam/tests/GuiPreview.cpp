#include <iostream>
#include "rpi_motioncam/RPiMotionCam.h"
#include <chrono>
#include <thread>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace rpi_motioncam;
using namespace cv;

RPIMOTIONCAM_OPTION_S options = RPiMotionCam::createMotionCamDefaultOptions();
bool running = true;
Mat img = Mat::zeros(600, 800, CV_8U);

void consume_regions() {
    cout << "Region consumer thread started" << endl;
    auto last_time = std::chrono::system_clock::now();
    int bytes = 0;
    float width_scale = 800.0 / options.width;
    float height_scale = 600.0 / options.height;
    while (running) {
        shared_ptr< MotionRegion > region;
        if (MotionData::has_regions() && MotionData::get_region(region)) {
            MOTIONREGION_READ_LOCK(region);
            Rect destRect = Rect((int)(region->roi.x * width_scale), (int)(region->roi.y * height_scale), (int)(region->roi.width * width_scale), (int)(region->roi.height * height_scale));
            Mat dest = img(destRect);
            resize(*(region->imgPtr), dest, Size(), width_scale, height_scale);
            bytes += region->imgPtr->total();
        }
    }
    cout << "Region consumer thread ending" << endl;
}

int wait() {
    sigset_t waitset;
    int sig;
    int result = 0;

    sigemptyset( &waitset );
    sigaddset( &waitset, SIGUSR1 );
    pthread_sigmask( SIG_BLOCK, &waitset, NULL );
    result = sigwait ( &waitset, &sig );
    return 1;
}


int main(int argc, char** argv) {

    namedWindow("GuiPreview");
    imshow("GuiPreview", img);
    
    options.preview = false;
    options.motion_threshold = 40;
    //options.resizer_width = 1024;
    //options.resizer_height = 768;
    auto cam = RPiMotionCam::create(options);

    if (cam->start() == MMAL_SUCCESS) {
        cout << "Start success" << endl;
    } else {
        cout << "Start failed" << endl;
        return -1;
    }

    std::thread region_consumer_thread(consume_regions);

    while (waitKey(1) == -1) {
        imshow("GuiPreview", img);
    }

    //while (wait());

    cout << "User interrupt" << endl;

    running = false;

    region_consumer_thread.join();

    cout << "Region consumer thread stopped" << endl;

    cam->stop();

    cout << "Camera stopped" << endl;
}
