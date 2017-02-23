#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int main() {
	VideoCapture vcap(0);
	if (!vcap.isOpened()) {
		cout << "Could not open webcam " << endl;
		return -1;
	}
	int width = vcap.get(CV_CAP_PROP_FRAME_WIDTH);
	int height = vcap.get(CV_CAP_PROP_FRAME_HEIGHT);
	VideoWriter video("out.avi", CV_FOURCC('M','J','P','G'), 60, Size(width, height), true);
	int frames = 0;
	for (;; frames++) {
		Mat frame;
		vcap >> frame;
		video.write(frame);
		imshow("Frame", frame);
		char c = (char)waitKey(1);
		if (c == 27 ) break;
	}
	cout << "Captured " << frames << " frames " << endl;
	return 0;
}
