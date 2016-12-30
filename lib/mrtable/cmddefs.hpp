#ifndef __CMDDEFS_HPP__ 
#define __CMDDEFS_HPP__

/**
 * CMD_PUT_REGION
 *
 * Create touch region
 *
 * Params: x y width height
 * Response: Region ID
 */
#define CMD_PUT_REGION      100

/**
 * CMD_UPDATE_REGION
 *
 * Update region dimensions
 *
 * Params: id x y width height
 * Response: OK
 */
#define CMD_UPDATE_REGION   101

/**
 * CMD_DELETE_REGION
 *
 * Delete existing touch region
 *
 * Params: id
 * Response: OK
 */
#define CMD_DELETE_REGION   102

#endif
