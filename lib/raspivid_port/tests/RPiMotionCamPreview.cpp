#include <iostream>
#include "MotionData.h"
#include "RaspiVid.h"
#include <chrono>
#include <thread>

using namespace std;
using namespace raspivid;

bool running = true;

void consume_frames() {
    cout << "Frame consumer thread started" << endl;
    auto last_time = std::chrono::system_clock::now();
    int bytes = 0;
    int frames = 0;
    int regions = 0;
    int seconds = 0;
    while (running) {
        if (MotionData::has_ready_frame()) {
            while (MotionData::has_ready_frame()) {
                frames++;
                shared_ptr< MotionData > frame;
                MotionData::get_ready_frame(frame);
                regions += frame->regions->size();
                for (auto region = frame->regions->begin(); region != frame->regions->end(); ++region) {
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
    RASPIVID_OPTIONS_S options = RaspiVid::createRaspiVidDefaultOptions();
    options.preview = true;
    options.motion_threshold = 200;
    //options.resizer_width = 1024;
    //options.resizer_height = 768;
    auto cam = RaspiVid::create(options);

    if (cam->init() == MMAL_SUCCESS) {
        cout << "Init success" << endl;
    } else {
        cout << "Init failed" << endl;
        return -1;
    }

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
