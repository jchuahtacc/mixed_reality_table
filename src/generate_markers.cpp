#include <iostream>
#include <opencv2/aruco.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>

using namespace cv;
using namespace std;

int main(int argc, char **argv) {
    
    vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);
    cv::Mat1b markerImage;
    Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_100);
    char filename[20] = { 0 };
    try {
        for (int i = 0; i < 100; i++) {
            sprintf(filename, "marker%03d.png", i);
            cv::aruco::drawMarker(dictionary, i, 32, markerImage, 1);
            imwrite(filename, markerImage, compression_params);
        }
    } catch (runtime_error& ex) {
        cout << "Exception: " << ex.what() << endl;
        return 1;
    }
    return 0;
    // cv::imshow("img", markerImage);
    // cv::waitKey(0);
}
