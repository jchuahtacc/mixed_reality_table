#ifndef __IMAGESOURCE_HPP__
#define __IMAGESOURCE_HPP__

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "Source.hpp"
#include <string>

namespace mrtable {
    namespace sources {
        class ImageSource : public Source {
            public:
                int fps = 0;

                Mat img;

                ImageSource(string filename) {
                    img = cv::imread(filename);    
                    getProps();
                }

                bool getFrame(Mat image) {
                    image = img;
                    return true;
                }

            private:
                bool opened = true;

                void getProps() {
                    width = img.cols;
                    height = img.rows;
                }
        };
    }
}

#endif
