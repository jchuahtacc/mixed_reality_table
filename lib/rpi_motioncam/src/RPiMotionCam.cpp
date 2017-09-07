#include "bcm_host.h"
#include "interface/vcos/vcos.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"

#include "../include/RPiMotionCam.h"
#include <iostream>

using namespace std;

namespace rpi_motioncam {
    RPiMotionCam::RPiMotionCam(int width, int height) : width_(width), height_(height) {
        cout << "Initializing RPiMotionCam at " << width_ << "x" << height_ << endl;
        
    }

    RPiMotionCam::~RPiMotionCam() {
    }
}
