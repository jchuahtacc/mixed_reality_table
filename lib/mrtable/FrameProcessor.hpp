#ifndef __FRAMEPROCESSOR_HPP__
#define __FRAMEPROCESSOR_HPP__

#include <opencv2/core/mat.hpp>
#include "result_t.hpp"
#include <chrono>

using namespace std::chrono;
using namespace cv;

namespace mrtable {
    namespace process {
        class FrameProcessor {
            public:
                virtual bool process(Mat input, Mat output, result_t& result) { return false; }
                
                string err = "Default error";
                string processor = "Virtual processor";

                result_t run(Mat input, Mat output, result_t& result) {
                    steady_clock::time_point detectBegin = steady_clock::now();
                    bool ok = process(input, output, result);
                    steady_clock::time_point detectEnd = steady_clock::now();
                    long int elapsed = duration_cast<milliseconds>(detectEnd - detectBegin).count();
                    if (!ok) {
                        std::cerr << "Processor " << processor << " failed after " << elapsed << " milliseconds with error: " << err << std::endl;
                    }
                    result.elapsed += (int)elapsed;
                    return result;
                }
        };
    }
}

#endif
