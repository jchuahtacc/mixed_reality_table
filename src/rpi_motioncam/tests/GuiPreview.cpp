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

void consume_frames(shared_ptr< RPiMotionCam > cam) {
    cout << "Frame consumer thread started" << endl;
    auto last_time = std::chrono::system_clock::now();
    int bytes = 0;
    int frames = 0;
    int regions = 0;
    int seconds = 0;
    float width_scale = 800.0 / options.width;
    float height_scale = 600.0 / options.height;
    while (running) {
        if (MotionData::has_ready_frames()) {
            while (MotionData::has_ready_frames()) {
                // frames++;
                MotionFrame frame;
                if (MotionData::get_ready_frame(frame)) {
                    regions += frame.regions.size();
                    for (auto it = frame.regions.begin(); it != frame.regions.end(); ++it) {
                        shared_ptr< MotionRegion > region = *it;
                        MOTIONREGION_READ_LOCK(region);
                        Rect destRect = Rect((int)(region->roi.x * width_scale), (int)(region->roi.y * height_scale), (int)(region->roi.width * width_scale), (int)(region->roi.height * height_scale));
                        Mat dest = img(destRect);
                        resize(*(region->imgPtr), dest, Size(), width_scale, height_scale);
                        bytes += region->imgPtr->total();
                    }
                    /*
                    if (std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now() - last_time ).count() > 1000) {
                        seconds++;
                        last_time = std::chrono::system_clock::now();
                        // cout << bytes << " bytes consumed from " << regions << " regions across " << frames << " frames" << endl;
                        bytes = 0;
                        frames = 0;
                        regions = 0;
                    }
                    */
                }
            }
        }
    }
    cout << "Frame consumer thread ending" << endl;
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

    std::thread frame_consumer_thread(consume_frames, cam);

    while (waitKey(1) == -1) {
        imshow("GuiPreview", img);
    }

    //while (wait());

    cout << "User interrupt" << endl;

    running = false;

    frame_consumer_thread.join();

    cout << "Frame consumer thread stopped" << endl;

    cam->stop();

    cout << "Camera stopped" << endl;
}
