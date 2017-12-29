#ifndef __MRTABLE_H__
#define __MRTABLE_H__

#include "tbb/concurrent_queue.h"
#include <memory>
#include <tuio/TuioContainer.h>

using namespace std;
using namespace tbb;

typedef concurrent_queue< shared_ptr< TUIO::TuioContainer > > ContainerPtrQueue;

#endif
