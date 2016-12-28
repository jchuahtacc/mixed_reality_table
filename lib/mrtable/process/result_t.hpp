#ifndef __RESULT_T_HPP__
#define __RESULT_T_HPP__

#include <map>
#include <iostream>

namespace mrtable {
    namespace process {
        typedef struct result_t {
            int frames;
            int detected;
            int elapsed;
            std::map<int, void* > outputs;
        } result_t;

        basic_ostream<char>& operator<<(basic_ostream<char>& outs, result_t& result) {
            outs << "Result: " << result.frames << " frames in " << result.elapsed << "ms (" << result.frames * 1000.0 / result.elapsed;
            outs << " fps) with " << result.detected << " detected objects";
            return outs;
        }
    }
}

#endif


