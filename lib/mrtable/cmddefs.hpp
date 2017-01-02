#ifndef __CMDDEFS_HPP__ 
#define __CMDDEFS_HPP__

/**
 * CMD_ECHO
 *
 * Echo test for server
 *
 * Params: message_to_echo
 */
#define CMD_ECHO            001

/**
 * CMD_PUT_REGION
 *
 * Create touch region
 *
 * Params: region_name x y width height
 */
#define CMD_PUT_REGION      100

/**
 * CMD_UPDATE_REGION
 *
 * Update region dimensions
 *
 * Params: region_name x y width height
 */
#define CMD_UPDATE_REGION   101

/**
 * CMD_DELETE_REGION
 *
 * Delete existing touch region
 *
 * Params: region_name 
 */
#define CMD_DELETE_REGION   102

/**
 * CMD_VERIFY_MARKER_PLACEMENT
 *
 * Take a snapshot of the current camera and verify bounds detection markers
 *
 * Params: None
 */
#define CMD_VERIFY_MARKER_PLACEMENT     200

/**
 * CMD_CALCULATE_ROI
 *
 * If marker placement was okay, go ahead and calculate camera bounds and orientation
 *
 * Params: None
 */
#define CMD_CALCULATE_ROI               201

/**
 * CMD_WRITE_BOUNDS
 *
 * Write current camera bounds/orientation to file specified in server config
 */
#define CMD_WRITE_BOUNDS                202

/**
 * CMD_READ_BOUNDS
 *
 * Read camera bounds/orientation from file specified in server config
 */
#define CMD_READ_BOUNDS                 203

#endif
