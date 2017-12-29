#ifndef __TESTPROCESSOR_HPP__
#define __TESTPROCESSOR_HPP__

#include "FrameProcessor.hpp"
#include <mrtable/data/data.hpp>
#include <opencv2/core/core.hpp>
#include <tuio/TuioServer.h>
#include <string>
#include <iostream>

using namespace mrtable::process;
using namespace mrtable::data;
using namespace cv;
using namespace std;

namespace mrtable {
    namespace process {
        class TestProcessor : public FrameProcessor {
            public:
                int procId;
                float xpos = 0.5;
                float ypos = 0.5;
                TestProcessor(int id) {
                    err = to_string(id);
                    processor = "Test processor";
                    procId = id;
                }

                ~TestProcessor() {
                }

                bool process(Mat& image, Ptr< SharedData >& data, result_t& result) {
                    Ptr< TUIO::TuioServer > server = data->server;
                    if (server != NULL) {
                        server->initFrame(TUIO::TuioTime::getSessionTime());

                        server->commitFrame();
                    }
                    return true;
                }

                static Ptr<FrameProcessor> create(int id) {
                    return makePtr<TestProcessor>(id).staticCast<FrameProcessor>();
                }

        };
    }
}

#endif
