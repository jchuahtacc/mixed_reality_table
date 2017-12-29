#ifndef __OUTPUTFUNCTOR_H__
#define __OUTPUTFUNCTOR_H__

#include <memory>
#include "ImgRecord.h"

using namespace std;

namespace mrtable_process {
    class OutputFunctor {
        public:
            void operator()(const shared_ptr< ImgRecord > input);
    };
}

#endif
