#include "RPiMotionCam.h"
#include <iostream>
#include "interface/mmal/mmal_types.h"

using namespace std;
using namespace rpi_motioncam;

int main(int argc, char** argv) {
    RPiMotionCam* cam = RPiMotionCam::create();
    MMAL_STATUS_T status = cam->init();
    if (status == MMAL_SUCCESS) {
        cout << "Success" << endl;
    } else {
        cout << "Failure with error " << status << endl;
    }
}
