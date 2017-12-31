#ifndef __RPIMOTIONCAM_TYPEDEFS_H__
#define __RPIMOTIONCAM_TYPEDEFS_H__

#include <stdint.h>

namespace rpi_motioncam {
    // Forward
    typedef struct RASPIVID_STATE_S RASPIVID_STATE;

    /** Possible raw output formats
     */
    typedef enum {
       RAW_OUTPUT_FMT_YUV = 1,
       RAW_OUTPUT_FMT_RGB,
       RAW_OUTPUT_FMT_GRAY,
    } RAW_OUTPUT_FMT;

    /** Structure containing all state information for the current run
     */
    typedef struct 
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
       
       bool preview;

    } RPIMOTIONCAM_OPTION_S;

}

#endif
