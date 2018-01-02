#include "typedefs.h"
#include "RegionCallback.h"
#include <memory>

using namespace std;

namespace rpi_motioncam {
    /** Structure containing all state information for the current run
     */
    struct RPIMOTIONCAM_OPTION
    {
       uint32_t width;                          /// Requested width of image
       uint32_t height;                         /// requested height of image
       uint32_t framerate;                      /// Requested frame rate (fps)
       bool verbose;                        /// !0 if want detailed run information

       RAW_OUTPUT_FMT raw_output_fmt;       /// The raw video format
       int cameraNum;                       /// Camera number
       int sensor_mode;			            /// Sensor mode. 0=auto. Check docs/forum for modes selected by other values.

       uint32_t resizer_width;               /// Resizer component width for compressed motion vectors
       uint32_t resizer_height;              /// Resizer component height 
       int motion_threshold;
       shared_ptr< RegionCallback > region_callback;
       
       bool preview;

    };

}
