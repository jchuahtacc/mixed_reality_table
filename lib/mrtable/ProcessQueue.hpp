#ifndef __PROCESSQUEUE_HPP__
#define __PROCESSQUEUE_HPP__

#include "ServerConfig.hpp"
#include "FrameProcessor.hpp"
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
                ProcessQueue(Ptr<ServerConfig> conf) {
                    config = conf;
                }

                ~ProcessQueue() {
                    config.release();
                }

                void addProcessor(Ptr<FrameProcessor> processor) {
                    processor->init(config);
                    processors.push_back(processor);
                }

                result_t process(Mat input, Mat output) {
                    result_t result;
                    result.frames = 1;
                    result.elapsed = 0;
                    result.detected = 0;
                    for(std::vector< Ptr<FrameProcessor> >::iterator it = processors.begin(); it != processors.end(); ++it) {
                        (*it)->run(input, output, result);
                    }
                    return result;
                }

            private:
                Ptr<ServerConfig> config;
                std::vector< Ptr<FrameProcessor> > processors;
        };
    }
}

#endif
