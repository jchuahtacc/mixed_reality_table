#include <iostream>
#include "RaspiVid.h"

using namespace std;
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
    MMAL_STATUS_T status = raspivid::init();
    /*
    RPiMotionCam* cam = RPiMotionCam::create();
    MMAL_STATUS_T status = cam->init(PICAM_DEFAULT, true);
    */
    if (status == MMAL_SUCCESS) {
        cout << "Init success" << endl;
    } else {
        cout << "Failure with error " << status << endl;
        return -1;
    }
    status = raspivid::start();
    if (status != MMAL_SUCCESS) {
        cout << "Start failed" << endl;
        return -1;
    }

    while (wait());
    
    raspivid::destroy();
}
