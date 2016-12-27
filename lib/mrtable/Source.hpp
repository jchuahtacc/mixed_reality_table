#ifndef __SOURCE_HPP__
#define __SOURCE_HPP__

#include <opencv2/core/core.hpp>

namespace mrtable {
    namespace sources {
        class Source {
            public:
                int width = 0;
                int height = 0;
                int waitTime = 0;
                
                virtual bool getFrame(Mat image) {
                    return false;
                }
        };
    }
}

#endif
