#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "components/RaspiOverlayRenderer.h"

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
    RaspiOverlayRenderer* renderer = RaspiOverlayRenderer::create();
    vcos_assert(renderer);
    cout << "Init success" << endl;
    int i = 0;
    MMAL_BUFFER_HEADER_T *buffer;
    while (wait()) {
        buffer = renderer->get_buffer();;

        memset(buffer->data, (i<<4)&0xff, buffer->alloc_size);
        buffer->length = buffer->alloc_size;

        renderer->send_buffer(buffer);
        i = (i + 1) % 10;
        sleep(1);
    }
    renderer->destroy();
    delete renderer;
}
