#include <iostream>
#include <rpi_motioncam/RPiMotionCam.h>
#include "tbb/concurrent_queue.h"
#include "mrtable_tuio/mrtable_tuio.h"
#include "config/config.h"
#include "process/process.h"
#include "mrtable.h"
#include <tuio/TuioServer.h>
#include <memory>
#include <thread>
#include <chrono>

using namespace std;
using namespace tbb;
using namespace mrtable_process;
using namespace rpi_motioncam;

shared_ptr< ContainerPtrQueue> container_queue = shared_ptr< ContainerPtrQueue >( new ContainerPtrQueue() );

class ImgProcessorCallback : public RegionCallback {
    public:
        shared_ptr< ImgProcessor > processor;
        ImgProcessorCallback(shared_ptr< ImgProcessor > processor_) : processor(processor_) {
        }
        void process(shared_ptr< MotionRegion > region) {
            processor->put(region);
        }
};

bool running = true;

void transport_callback(shared_ptr< ImgProcessor > processor) {
    while (running) {
        shared_ptr< MotionRegion > region;
        if (MotionData::has_regions() && MotionData::get_region(region)) {
            if (!processor->put(region)) {
                cout << "Couldn't process region!" << endl;
            }
        }
    }
    cout << "Transport thread ending" << endl;
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


int main(int argc, const char* argv[]) {
    cout << "Initializing TUIO Server" << endl;
    mrtable_tuio::TuioWrapper::init(container_queue);
    cout << "Initialized TUIO Server" << endl;

    auto processor = shared_ptr< ImgProcessor >( new ImgProcessor() );

    //std::thread transport_thread(transport_callback, processor);

    cout << "Starting RPiMotionCam" << endl;
    auto options = RPiMotionCam::createMotionCamDefaultOptions();
    options.motion_threshold = 80;
    options.region_callback = shared_ptr< ImgProcessorCallback >( new ImgProcessorCallback( processor ) );

    auto cam = RPiMotionCam::create(options);
    if (cam->start() == MMAL_SUCCESS) {
        cout << "RPiMotionCam started" << endl;
    } else {
        cout << "RPiMotionCam failed to start" << endl;
    }

    while(wait());

    cout << "Shutting down" << endl;
    running = false;

    // transport_thread.join();

}
