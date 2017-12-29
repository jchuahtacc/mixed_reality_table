#include "OutputFunctor.h"

#include <iostream>

using namespace std;

namespace mrtable_process {
    void OutputFunctor::operator() (const shared_ptr< ImgRecord > input) {
        cout << "Output functor" << endl;
    }
}
