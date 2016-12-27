#ifndef __RESULT_T_HPP__
#define __RESULT_T_HPP__

#include <map>

namespace mrtable {
    namespace process {
        typedef struct result_t {
            int frames;
            int detected;
            int elapsed;
            std::map<int, void* > outputs;
        } result_t;
    }
}

#endif


