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
                TestProcessor(int id) {
                    err = to_string(id);
                    processor = "Test processor";
                    procId = id;
                    SharedData::put(procId, &procId);
                    server = SharedData::getPtr<TUIO::TuioServer>(KEY_TUIO_SERVER);
                }

                ~TestProcessor() {
                    SharedData::erase(procId);
                }

                bool process(Mat& image, result_t& result) {
                    if (server != NULL) {
                        if (obj == NULL) {
                            obj = server->addTuioObject(1, 0.5, 0.5, 0.0);
                        } else {
                            server->updateTuioObject(obj, 0.5, 0.5, 0.0);
                        }
                    }
                    return true;
                }

                static Ptr<FrameProcessor> create(int id) {
                    return makePtr<TestProcessor>(id).staticCast<FrameProcessor>();
                }

            private:
                TUIO::TuioServer* server = NULL;
                TUIO::TuioObject* obj = NULL;
        };
    }
}

#endif
