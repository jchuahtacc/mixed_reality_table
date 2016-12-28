#ifndef __TESTPROCESSOR_HPP__
#define __TESTPROCESSOR_HPP__

#include "FrameProcessor.hpp"
#include <opencv2/core/core.hpp>
#include <string>

using namespace mrtable::process;
using namespace cv;
using namespace std;

namespace mrtable {
    namespace process {
        class TestProcessor : public FrameProcessor {
            public:
                int procId;
                TestProcessor(int id) {
                    err = to_string(id);
                    processor = "Test processor";
                    //procId = new int;
                    procId = id;
                }

                ~TestProcessor() {
                    outputs->erase(procId);
                }

                void init(Ptr<ServerConfig> config) {
                    outputs->put(procId, &procId);
                }

                bool process(Mat& image, result_t& result) {
                    //result.outputs[*procId] = procId;
                    return false;
                }

                static Ptr<FrameProcessor> create(int id) {
                    return makePtr<TestProcessor>(id).staticCast<FrameProcessor>();
                }
        };
    }
}

#endif
