#include "OutputFunctor.h"

#include <iostream>

using namespace std;

namespace mrtable_process {
    void OutputFunctor::operator() (const RegionRecord &input) {
        cout << "Output functor" << endl;
    }
}
