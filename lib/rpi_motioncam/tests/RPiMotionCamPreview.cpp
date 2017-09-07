#include "RPiMotionCam.h"
#include <iostream>

using namespace std;
using namespace rpi_motioncam;

int main(int argc, char** argv) {
    RPiMotionCam cam = RPiMotionCam();

    cout << "Init Status " << cam.init() << endl;
}
