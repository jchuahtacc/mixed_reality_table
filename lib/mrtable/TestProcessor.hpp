#ifndef __TESTPROCESSOR_HPP__
#define __TESTPROCESSOR_HPP__

#include "mrtable.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace mrtable::process;
using namespace cv;
using namespace std;

namespace mrtable {
    namespace process {
        class TestProcessor : public FrameProcessor {
            public:
                int* procId;
                TestProcessor(int id) {
                    err = to_string(id);
                    processor = "Test processor";
                    procId = new int;
                    *procId = id;
                }

                ~TestProcessor() {
                }

                bool process(Mat& input, Mat& output, result_t& result) {
                    result.outputs[*procId] = procId;
                    return false;
                }
        };
    }
}

#endif
