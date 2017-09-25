#include <iostream>
#include "RaspiVid.h"

using namespace std;
using namespace raspivid;

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
    RASPIVID_STATE_S state = createRaspiVidDefaultState();
    state.preview = true;
    RaspiVid* cam = RaspiVid::create(state);

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

    while (wait());

    cout << "User interrupt" << endl;

    cam->stop();

    cout << "Camera stopped" << endl;
}
