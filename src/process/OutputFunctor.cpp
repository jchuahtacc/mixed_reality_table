#include "OutputFunctor.h"

#include <iostream>
#include <chrono>

using namespace std;

namespace mrtable_process {
    OutputFunctor::OutputFunctor() {
        start = system_clock::now();
    }
    void OutputFunctor::operator() (const RegionRecord &input) {
        std::chrono::time_point<std::chrono::system_clock> aruco_start;
        std::chrono::time_point<std::chrono::system_clock> aruco_finish;

        if (input.region->get_event("buffered", aruco_start) && input.region->get_event("aruco_finish", aruco_finish)) {
            aruco_duration += duration_cast<milliseconds>( aruco_finish - aruco_start );
            aruco_count++;
        }

        auto now = system_clock::now();
        if (duration_cast<milliseconds>( now - start ).count() > 1000) {
            start = now;
            if (aruco_count > 0) {
                cout << "Aruco regions processed: " << aruco_count << " with average latency " << (aruco_duration.count() * 1.0 / aruco_count) << "ms" << endl;
                aruco_count = 0;
                aruco_duration = milliseconds::zero();
            }
        }
    }
}
