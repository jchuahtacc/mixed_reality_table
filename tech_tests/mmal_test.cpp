#include <interface/mmal/mmal.h>
#include <interface/mmal/mmal_component.h>
#include <interface/mmal/util/mmal_default_components.h>
#include <iostream>


using namespace std;


void ABORT_IF_ERROR(MMAL_STATUS_T status) {
    switch(status) {
        case MMAL_SUCCESS : return;
        case MMAL_ENOMEM : {
                               cout << "MMAL_STATUS_T Error: MMAL_ENOMEM (Out of Memory)" << endl; exit(-1);
                           }
        case MMAL_ENOSPC : {
                               cout << "MMAL_STATUS_T Error: MMAL_ENOSPC (Out of resources)" << endl; exit(-1);
                           }
        case MMAL_EINVAL : {
                               cout << "MMAL_STATUS_T Error: MMAL_EINVAL (Argument is invalid)" << endl; exit(-1);
                           }
        case MMAL_ENOSYS : {
                               cout << "MMAL_STATUS_T Error: MMAL_ENOSYS (Function not implemented)" << endl; exit(-1);
                           }
        case MMAL_ENOENT : {
                               cout << "MMAL_STATUS_T Error: MMAL_ENOENT (No such file or directory)" << endl; exit(-1);
                           }
        case MMAL_ENXIO : {
                               cout << "MMAL_STATUS_T Error: MMAL_ENXIO (No such device or address)" << endl; exit(-1);
                           }
        case MMAL_EIO : {
                               cout << "MMAL_STATUS_T Error: MMAL_EIO (I/O Error)" << endl; exit(-1);
                           }
        case MMAL_ESPIPE : {
                               cout << "MMAL_STATUS_T Error: MMAL_ESPIPE (Illegal seek)" << endl; exit(-1);
                           }
        case MMAL_ECORRUPT : {
                               cout << "MMAL_STATUS_T Error: MMAL_ECORRUPT (Data is corrupt)" << endl; exit(-1);
                           }
        case MMAL_ENOTREADY : {
                               cout << "MMAL_STATUS_T Error: MMAL_ENOTREADY (Component is not ready)" << endl; exit(-1);
                           }
        case MMAL_EISCONN : {
                               cout << "MMAL_STATUS_T Error: MMAL_EISCONN (Port is already connected)" << endl; exit(-1);
                           }
        case MMAL_EAGAIN : {
                               cout << "MMAL_STATUS_T Error: MMAL_EAGAIN (Resource temporarily unavailable. Try again later.)" << endl; exit(-1);
                           }
        case MMAL_EFAULT : {
                               cout << "MMAL_STATUS_T Error: MMAL_EFAULT (Bad address)" << endl; exit(-1);
                           }
        case MMAL_STATUS_MAX : {
                               cout << "MMAL_STATUS_T Error: MMAL_STATUS_MAX (Force to 32 bit)" << endl; exit(-1);
                           }
        default : {
                               cout << "MMAL_STATUS_T Error: (Unknown!)" << endl; exit(-1);
                           }
 
    }
}


MMAL_COMPONENT_T *camera = 0;
MMAL_STATUS_T  status;

int main(int argc, char** argv) {
    cout << "Creating camera" << endl;
    // Make the camera component
    status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);
    ABORT_IF_ERROR(status);

    MMAL_PARAMETER_INT32_T camera_num = {{MMAL_PARAMETER_CAMERA_NUM, sizeof(camera_num)}, 0};
}
