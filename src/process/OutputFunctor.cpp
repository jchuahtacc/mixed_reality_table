#include "OutputFunctor.h"

#include <iostream>

using namespace std;

namespace mrtable_process {
    void OutputFunctor::operator() (const int input) {
        cout << "Output functor: " << input << endl;
    }
}
