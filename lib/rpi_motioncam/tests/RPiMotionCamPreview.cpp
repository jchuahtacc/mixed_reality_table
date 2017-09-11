#include "RPiMotionCam.h"
#include <iostream>
#include "interface/mmal/mmal_types.h"

using namespace std;
using namespace rpi_motioncam;

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
    RPiMotionCam* cam = RPiMotionCam::create();
    MMAL_STATUS_T status = cam->init(PICAM2X_640_480_90, true);
    if (status == MMAL_SUCCESS) {
        cout << "Init success" << endl;
    } else {
        cout << "Failure with error " << status << endl;
        return -1;
    }
    status = cam->start();
    if (status != MMAL_SUCCESS) {
        cout << "Start failed" << endl;
        return -1;
    }

    while (wait());
}
