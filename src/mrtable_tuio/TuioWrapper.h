#ifndef __TUIOWRAPPER_H__
#define __TUIOWRAPPER_H__

#include <thread>
#include <tuio/TuioContainer.h>
#include <tuio/TuioServer.h>
#include "../mrtable.h"

namespace mrtable_tuio {
    class TuioWrapper {
        public:
            static bool init(shared_ptr< ContainerPtrQueue > update_queue);
            static void put(shared_ptr< TUIO::TuioContainer > container);
            static void worker();
            static void stop();
        private:
            static shared_ptr< TUIO::TuioServer > tuio_server;
            static shared_ptr< ContainerPtrQueue > queue;
            static bool running;
    };
}

#endif
