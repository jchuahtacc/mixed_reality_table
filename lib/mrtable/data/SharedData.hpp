#include <opencv2/core/core.hpp>
#include <tuio/TuioServer.h>
#include <vector>
#include <map>
#include "../process/Marker.h"
#include "../process/Touch.h"

using namespace cv;
using namespace std;
using namespace mrtable::process;

namespace mrtable {
    namespace data {
        class SharedData {
            public:
                SharedData() {
                    regionMessages = makePtr< vector< Message > >();
                }

                static Ptr<SharedData> create() {
                    return makePtr<SharedData>();
                }

                Ptr< TUIO::TuioServer > server;

                vector< int > ids;
                vector< vector< Point2f > > corners;
                vector< vector< Point2f > > rejected;
                vector< Marker > markers;
                double otsu_std_dev = 0;
                map< string, Ptr< vector< Vec4i > > > hierarchies;
                map< string, Ptr< Rect_<float > > > regions;
                Ptr< vector< Message > > regionMessages;
                map< string, Ptr < vector< vector<Point> > > > contours;
                vector< vector<Point> > globalContours;
                vector< Vec4i > globalHierarchy;
                vector< Touch > globalTouches;
                map< string, Ptr < vector< Touch > > > touches;
                int keyPress = 0;
                
        };
    }
}
