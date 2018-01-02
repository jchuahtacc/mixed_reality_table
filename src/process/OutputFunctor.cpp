#include "OutputFunctor.h"

#include <iostream>
#include <chrono>

using namespace std;

namespace mrtable_process {
    OutputFunctor::OutputFunctor() {
        start = system_clock::now();
    }
    void OutputFunctor::operator() (const RegionRecord &input) {
        region_count++;
        byte_count += input.region->imgPtr->total();
        std::chrono::time_point<std::chrono::system_clock> buffer_start;

        if (input.region->get_event("buffered", buffer_start)) {
            latencies += duration_cast<milliseconds>( system_clock::now() - buffer_start );
        }

        auto now = system_clock::now();
        if (duration_cast<milliseconds>( now - start ).count() > 1000) {
            start = now;
            if (region_count > 0) {
                cout << "Regions: " << region_count << " bytes: " << byte_count << " latency: " << (latencies.count() * 1.0 / region_count) << "ms" << endl;
                region_count = 0;
                byte_count = 0;
                latencies = milliseconds::zero();
            }
        }
    }
}
