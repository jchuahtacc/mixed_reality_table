#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <memory>
#include "components/RaspiOverlayRenderer.h"

using namespace std;
using namespace raspivid;


int main(int argc, char** argv) {
    RASPIOVERLAYRENDERER_FORMAT_S format = RaspiOverlayRenderer::createDefaultOverlayFormat();
    format.alpha = 128;
    format.width = 640;
    format.height = 480;
    auto renderer = RaspiOverlayRenderer::create(format);
    vcos_assert(renderer);
    cout << "Init success" << endl;
    int i = 0;
    MMAL_BUFFER_HEADER_T *buffer;
    for (int i = 0; i < 10; i++) {
        cout << "getting buffer" << endl;
        buffer = renderer->get_buffer();

        cout << "got buffer" << endl;
        memset(buffer->data, (i<<4)&0xff, buffer->alloc_size);
        buffer->length = buffer->alloc_size;

        cout << "Sending buffer of size " << buffer->alloc_size << endl;

        renderer->send_buffer(buffer);
        sleep(1);
    }
}
