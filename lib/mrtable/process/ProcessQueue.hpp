#ifndef __PROCESSQUEUE_HPP__
#define __PROCESSQUEUE_HPP__

#include "../config/ServerConfig.hpp"
#include "FrameProcessor.hpp"
#include "ProcessorOutput.hpp"
#include "result_t.hpp"
#include <opencv2/core/mat.hpp>
#include <opencv2/core/core.hpp>
#include <string>
#include <vector>

using namespace mrtable::process;
using namespace cv;

namespace mrtable {
    namespace process {
        class ProcessQueue {
            public:
                ProcessQueue(Ptr<ServerConfig> conf, Ptr<ProcessorOutput> output) {
                    config = conf;
                    outputs = output;
                }

                ~ProcessQueue() {
                    config.release();
                }

                void addProcessor(Ptr<FrameProcessor> processor) {
                    processor->init(config, outputs);
                    processors.push_back(processor);
                }

                result_t process(Mat& image) {
                    result_t result;
                    result.frames = 1;
                    result.elapsed = 0;
                    result.detected = 0;
                    for(std::vector< Ptr<FrameProcessor> >::iterator it = processors.begin(); it != processors.end(); ++it) {
                        (*it)->run(image, result);
                    }
                    return result;
                }

            private:
                Ptr<ServerConfig> config;
                Ptr<ProcessorOutput> outputs;
                std::vector< Ptr<FrameProcessor> > processors;
        };
    }
}

#endif
