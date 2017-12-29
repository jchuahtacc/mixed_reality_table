#include "ImgRecord.h"

using namespace std;
using namespace rpi_motioncam;

namespace mrtable_process {
    ImgRecord::ImgRecord(shared_ptr< MotionData > motion_data_) : motion_data(motion_data_) {
        events.push_back({std::chrono::system_clock::now(), "start"});
    }
}
