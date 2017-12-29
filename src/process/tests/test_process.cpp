#include "../process.h"

#include <memory>


using namespace std;
using namespace mrtable_process;

int main(int argc, const char *argv[]) {
    auto processor = shared_ptr< ImgProcessor >( new ImgProcessor() );
    processor->put(3);
}
