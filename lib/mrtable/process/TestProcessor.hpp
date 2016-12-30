#ifndef __TESTPROCESSOR_HPP__
#define __TESTPROCESSOR_HPP__

#include "FrameProcessor.hpp"
#include <mrtable/data/data.hpp>
#include <opencv2/core/core.hpp>
#include <string>

using namespace mrtable::process;
using namespace mrtable::data;
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
                    SharedData::erase(procId);
                }

                void init(Ptr<ServerConfig> config) {
                    //outputs->put(procId, &procId);
                    SharedData::put(procId, &procId);
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
