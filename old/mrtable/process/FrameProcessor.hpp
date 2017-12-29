#ifndef __FRAMEPROCESSOR_HPP__
#define __FRAMEPROCESSOR_HPP__

#include <opencv2/core/mat.hpp>
#include "result_t.hpp"
#include <chrono>
#include <string>

using namespace std::chrono;
using namespace cv;

namespace mrtable {
    namespace process {
        class FrameProcessor {
            public:
                ~FrameProcessor() {
                }
                virtual bool process(Mat& image, Ptr< SharedData >& data, result_t& result) { 
                    return false; 
                }

                string err = "Default error";
                string processor = "Virtual processor";

                result_t run(Mat& image, Ptr< SharedData >& data, result_t& result) {
                    steady_clock::time_point detectBegin = steady_clock::now();
                    bool ok = false;
                    try {
                        ok = process(image, data, result);
                    } catch (const std::exception& e) {
                        std::cerr << e.what() << std::endl;
                    }
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
