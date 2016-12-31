#ifndef __CMDDEFS_HPP__ 
#define __CMDDEFS_HPP__

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

#endif
