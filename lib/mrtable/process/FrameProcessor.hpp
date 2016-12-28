#ifndef __FRAMEPROCESSOR_HPP__
#define __FRAMEPROCESSOR_HPP__

#include <opencv2/core/mat.hpp>
#include "result_t.hpp"
#include <chrono>
#include "../config/ServerConfig.hpp"
#include "ProcessorOutput.hpp"
#include <string>

using namespace std::chrono;
using namespace cv;

namespace mrtable {
    namespace process {
        class FrameProcessor {
            public:
                virtual bool process(Mat& image, result_t& result) { 
                    return false; 
                }

                virtual void init(Ptr<ServerConfig> config, Ptr<ProcessorOutput> outputs) {
                }
                
                string err = "Default error";
                string processor = "Virtual processor";

                result_t run(Mat& image, result_t& result) {
                    steady_clock::time_point detectBegin = steady_clock::now();
                    bool ok = process(image, result);
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
