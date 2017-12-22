#include "MotionVectorCallback.h"
#include <iostream>

namespace raspivid {

    int MotionRegion::num_rows = 0;
    int MotionRegion::num_cols = 0;

    MotionRegion::MotionRegion(int row_, int col_) : row(row_), col(col_), width(1), height(1) {
        grow_up();
        grow_down();
        grow_left();
        grow_right();
    }

    bool MotionRegion::grow_up() {
        if (row > 0) {
            row = row - 1;
            height = height + 1;
            return true;
        }
        return false;
    }

    bool MotionRegion::grow_down() {
        if (row + height < num_rows) {
            height = height + 1;
            return true;
        }
        return false;
    }

    bool MotionRegion::grow_left() {
        if (col > 0) {
            col = col - 1;
            width = width + 1;
            return true;
        }
        return false;
    }

    bool MotionRegion::grow_right() {
        if (col + width < num_cols) {
            width = width + 1;
            return true;
        }
        return false;
    }

    MotionVectorCallback::MotionVectorCallback(int width, int height) : cols_(width / 16), rows_(height / 16), new_vectors(false) {
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
            if (!(searched[row * cols_ + region.col]) && buffer->data[buffer_pos(row, region.col)] > MOTION_THRESHOLD) {
                return region.grow_left();
            }
        }
        return false;
    }

    bool MotionVectorCallback::check_right(MMAL_BUFFER_HEADER_T *buffer, bool *searched, MotionRegion &region) {
        for (int row = region.row; row < region.row + region.height; row++) {
            if (!(searched[row * cols_ + region.col]) && buffer->data[buffer_pos(row, region.col + region.width - 1)] > MOTION_THRESHOLD) {
                return region.grow_right();
            }
        }
        return false;
    }

    bool MotionVectorCallback::check_top(MMAL_BUFFER_HEADER_T *buffer, bool *searched, MotionRegion &region) {
        for (int col = region.col; col < region.col + region.width; col++) {
            if (!(searched[region.row * cols_ + col]) && buffer->data[buffer_pos(region.row, col)] > MOTION_THRESHOLD) {
                return region.grow_up();
            }
        }
        return false;
    }

    bool MotionVectorCallback::check_bottom(MMAL_BUFFER_HEADER_T *buffer, bool *searched, MotionRegion &region) {
        for (int col = region.col; col < region.col + region.width; col++) {
            if (!(searched[region.row * cols_ + col]) &&buffer->data[buffer_pos(region.row + region.height - 1, col)] > MOTION_THRESHOLD) {
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
            /*
            bool result = false;
            result = check_left(buffer, searched, region);
            while (result) {
                result = check_left(buffer, searched, region);
            }
            growing = growing || result;
            result = check_top(buffer, searched, region);
            while (result) {
                result = check_top(buffer, searched, region);
            }
            growing = growing || result;
            result = check_right(buffer, searched, region);
            while (result) {
                result = check_right(buffer, searched, region);
            }
            growing = growing || result;
            result = check_bottom(buffer, searched, region);
            while (result) {
                result = check_bottom(buffer, searched, region);
            }
            growing = growing || result;
            */
        }
    }

    void MotionVectorCallback::callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
        if ((buffer->flags & MMAL_BUFFER_HEADER_FLAG_CODECSIDEINFO)) {
            regions.clear();
            for (int i = 0; i < rows_ * cols_; searched[i++] = false);
            for (int row = 0; row < rows_; row++) {
                for (int col = 0; col < cols_; col++) {
                    if (searched[row * cols_ + col]) {
                        break;
                    }
                    if (buffer->data[buffer_pos(row, col)] > MOTION_THRESHOLD) {
                        MotionRegion region = MotionRegion(row, col);

                        grow_region(buffer, searched, region);

                        for (int i = region.row; i < region.row + region.height; i++) {
                            for (int j = region.col; j < region.col + region.width; j++) {
                                searched[i * cols_ + j] = true;
                            }
                        }

                        regions.push_back(region);
                        
                    }
                }
            }
            new_vectors = true;
        }
    }

    void MotionVectorCallback::post_process() {
        if (new_vectors) {
            new_vectors = false;
        }
    }
}
