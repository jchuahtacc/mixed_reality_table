#include <iostream>
#include "rpi_motioncam/RPiMotionCam.h"
#include <chrono>
#include <thread>

using namespace std;
using namespace rpi_motioncam;

shared_ptr< RPiMotionCam > cam = nullptr; 

bool running = true;

void consume_frames() {
    cout << "Frame consumer thread started" << endl;
    auto last_time = std::chrono::system_clock::now();
    int bytes = 0;
    int frames = 0;
    int regions = 0;
    int seconds = 0;
    while (running) {
        if (MotionData::has_ready_frames()) {
            while (MotionData::has_ready_frames()) {
                frames++;
                MotionFrame frame;
                if (MotionData::get_ready_frame(frame)) {
                    regions += frame.regions.size();
                    for (auto it = frame.regions.begin(); it != frame.regions.end(); ++it) {
                        shared_ptr< MotionRegion > region = *it;
                        MOTIONREGION_READ_LOCK(region);
                        bytes += region->imgPtr->total();
                    }
                    if (std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now() - last_time ).count() > 1000) {
                        seconds++;
                        last_time = std::chrono::system_clock::now();
                        cout << bytes << " bytes consumed from " << regions << " regions across " << frames << " frames" << endl;
                        bytes = 0;
                        frames = 0;
                        regions = 0;
                    }
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

    shared_ptr< MotionRegion > mandatory_region = shared_ptr< MotionRegion >(new MotionRegion() );
    mandatory_region->allocate(cv::Rect(1, 1, 5, 5));
    MotionData::set_mandatory_region(mandatory_region);

    RPIMOTIONCAM_OPTION_S options = RPiMotionCam::createMotionCamDefaultOptions();
    options.preview = true;
    //options.motion_threshold = 220;
    //options.resizer_width = 1024;
    //options.resizer_height = 768;
    cam = RPiMotionCam::create(options);

    if (cam->start() == MMAL_SUCCESS) {
        cout << "Start success" << endl;
    } else {
        cout << "Start failed" << endl;
        return -1;
    }

    std::thread frame_consumer_thread(consume_frames);

    while (wait());

    cout << "User interrupt" << endl;

    running = false;

    frame_consumer_thread.join();

    cout << "Frame consumer thread stopped" << endl;

    cam->stop();

    cout << "Camera stopped" << endl;
}
