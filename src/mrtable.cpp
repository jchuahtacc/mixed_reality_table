#include <iostream>
#include <rpi_motioncam/RPiMotionCam.h>
#include "tbb/concurrent_queue.h"
#include "mrtable_tuio/mrtable_tuio.h"
#include "config/config.h"
#include "mrtable.h"
#include <tuio/TuioServer.h>
#include <memory>
#include <thread>
#include <chrono>

using namespace std;
using namespace tbb;

shared_ptr< ContainerPtrQueue> container_queue = shared_ptr< ContainerPtrQueue >( new ContainerPtrQueue() );

int main(int argc, const char* argv[]) {
    cout << "Initializing TUIO Server" << endl;
    mrtable_tuio::TuioWrapper::init(container_queue);
    cout << "Initialized TUIO Server" << endl;
    cout << "Shutting down" << endl;

}
