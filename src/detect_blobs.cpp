#include <opencv2/features2d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

int main(int argc, char** argv) {
    Mat im = imread("blob2.jpg", IMREAD_GRAYSCALE);

    SimpleBlobDetector::Params params = SimpleBlobDetector::Params::Params();
    params.filterByColor = 1;
    params.blobColor = 255;
    params.minThreshold = 10;
    params.maxThreshold = 200;
    //SimpleBlobDetector detector;
    Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(params);

    vector<KeyPoint> keypoints;
    detector->detect(im , keypoints);
    Mat output;
    drawKeypoints(im, keypoints, output, Scalar(0, 0, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    namedWindow("keypoints");
    imshow("keypoints", output);
    waitKey(0);
}
