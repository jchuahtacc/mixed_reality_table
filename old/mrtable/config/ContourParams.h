#ifndef __CONTOURPARAMS_H__
#define __CONTOURPARAMS_H__

#include <opencv2/core/core.hpp>
#include <iostream>
#include <string>
#include <memory>

using namespace std;
using namespace cv;

namespace mrtable {
    namespace config {
        class ContourParams {
            public:
                float minPointRatio = 0.2;
                float maxPointRatio = 0.4;
                float minWidthRatio = 0.05;
                float maxWidthRatio = 0.1;

                static shared_ptr<ContourParams> create(string filename = "");
                bool read(string filename);
                bool write(string filename);
                bool parse(cv::String& paramString);
                friend basic_ostream<char>& operator<<(basic_ostream<char>& outs, ContourParams params);
                friend basic_ostream<char>& operator<<(basic_ostream<char>& outs, shared_ptr<ContourParams> params);
                ~ContourParams();
            protected:
                ContourParams();
                ContourParams(string filename = "");
        };
    }
}


#endif
