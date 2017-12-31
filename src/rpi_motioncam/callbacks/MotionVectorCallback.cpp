#include "rpi_motioncam/callbacks/MotionVectorCallback.h"
#include <iostream>

namespace rpi_motioncam {

    MotionVectorCallback::MotionVectorCallback(RPIMOTIONCAM_OPTION_S options) : cols_(options.resizer_width / 16), rows_(options.resizer_height / 16), lastRegions(nullptr), threshold_(options.motion_threshold), width_(options.resizer_width), height_(options.resizer_height), options_(options) {
        MotionRegion::num_rows = rows_;
        MotionRegion::num_cols = cols_;
        int elements = rows_ * cols_;
        searched = new bool[elements];
    }

    MotionVectorCallback::~MotionVectorCallback() {
        delete searched;
    }

    int MotionVectorCallback::buffer_pos(int row, int col) {
        return (row * (cols_ + 1) + col) * 4 + 2;
    }

    bool MotionVectorCallback::check_left(MMAL_BUFFER_HEADER_T *buffer, bool *searched, MotionRegion &region) {
        for (int row = region.row; row < region.row + region.height; row++) {
            if (!(searched[row * cols_ + region.col]) && buffer->data[buffer_pos(row, region.col)] > threshold_) {
                return region.grow_left();
            }
        }
        return false;
    }

    bool MotionVectorCallback::check_right(MMAL_BUFFER_HEADER_T *buffer, bool *searched, MotionRegion &region) {
        for (int row = region.row; row < region.row + region.height; row++) {
            if (!(searched[row * cols_ + region.col]) && buffer->data[buffer_pos(row, region.col + region.width - 1)] > threshold_) {
                return region.grow_right();
            }
        }
        return false;
    }

    bool MotionVectorCallback::check_top(MMAL_BUFFER_HEADER_T *buffer, bool *searched, MotionRegion &region) {
        for (int col = region.col; col < region.col + region.width; col++) {
            if (!(searched[region.row * cols_ + col]) && buffer->data[buffer_pos(region.row, col)] > threshold_) {
                return region.grow_up();
            }
        }
        return false;
    }

    bool MotionVectorCallback::check_bottom(MMAL_BUFFER_HEADER_T *buffer, bool *searched, MotionRegion &region) {
        for (int col = region.col; col < region.col + region.width; col++) {
            if (!(searched[region.row * cols_ + col]) &&buffer->data[buffer_pos(region.row + region.height - 1, col)] > threshold_) {
                return region.grow_down();
            }
        }
        return false;
    }

    void MotionVectorCallback::grow_region(MMAL_BUFFER_HEADER_T *buffer, bool *searched, MotionRegion &region) {
        bool growing = true;
        while (growing) {
            growing = false;
            growing = check_left(buffer, searched, region) || growing;
            growing = check_top(buffer, searched, region) || growing;
            growing = check_right(buffer, searched, region) || growing;
            growing = check_bottom(buffer, searched, region) || growing;
        }
    }

    void MotionVectorCallback::callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
        if ((buffer->flags & MMAL_BUFFER_HEADER_FLAG_CODECSIDEINFO)) {
            // buffer_count++;
            // vcos_log_error("MotionVectorCallback::callback(): buffer #%d", buffer_count);
            lastRegions = nullptr;
            auto regions = shared_ptr< vector< MotionRegion > >( new vector< MotionRegion >() );
            for (int i = 0; i < rows_ * cols_; searched[i++] = false);
            for (int row = 0; row < rows_; row++) {
                for (int col = 0; col < cols_; col++) {
                    if (searched[row * cols_ + col]) {
                        break;
                    }
                    if (buffer->data[buffer_pos(row, col)] > threshold_) {
                        MotionRegion region = MotionRegion(row, col);

                        grow_region(buffer, searched, region);

                        for (int i = region.row; i < region.row + region.height; i++) {
                            for (int j = region.col; j < region.col + region.width; j++) {
                                searched[i * cols_ + j] = true;
                            }
                        }

                        regions->push_back(region);
                        
                    }
                }
            }
            if (regions->size()) {
                lastRegions = regions;
                MotionData::stage_regions( regions );
            }
        }
    }

    void MotionVectorCallback::post_process() {
    }
}
