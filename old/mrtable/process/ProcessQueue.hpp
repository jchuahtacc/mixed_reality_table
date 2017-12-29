#ifndef __PROCESSQUEUE_HPP__
#define __PROCESSQUEUE_HPP__

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
                ProcessQueue() {
                }

                ~ProcessQueue() {
                }

                void addProcessor(Ptr<FrameProcessor> processor) {
                    processors.push_back(processor);
                }

                result_t process(Mat& image, Ptr< SharedData >& data) {
                    result_t result;
                    result.frames = 1;
                    result.elapsed = 0;
                    result.detected = 0;
                    for(std::vector< Ptr<FrameProcessor> >::iterator it = processors.begin(); it != processors.end(); ++it) {
                        (*it)->run(image, data, result);
                    }
                    return result;
                }

            private:
                std::vector< Ptr<FrameProcessor> > processors;
        };
    }
}

#endif
