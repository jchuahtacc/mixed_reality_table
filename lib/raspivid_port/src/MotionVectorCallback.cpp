#include "MotionVectorCallback.h"

namespace raspivid {

    int MotionRegion::last_row = 0;
    int MotionRegion::last_col = 0;

    MotionRegion::MotionRegion(int row_, int col_) : row(row_), col(col_), width(0), height(0) {
        grow_up();
        grow_down();
        grow_left();
        grow_right();
    }

    bool MotionRegion::grow_up() {
        if (row > 0) {
            row = row - 1;
            return true;
        }
        return false;
    }

    bool MotionRegion::grow_down() {
        if (row + height < last_row) {
            height = height + 1;
            return true;
        }
        return false;
    }

    bool MotionRegion::grow_left() {
        if (col > 0) {
            col = col - 1;
            return true;
        }
        return false;
    }

    bool MotionRegion::grow_right() {
        if (col + width < last_col) {
            width = width + 1;
            return true;
        }
        return false;
    }

    MotionVectorCallback::MotionVectorCallback(int width, int height) : cols_(width / 16), rows_(height / 16), new_vectors(false) {
        MotionRegion::last_row = rows_ - 1;
        MotionRegion::last_row = cols_ - 1;
    }


    int MotionVectorCallback::buffer_pos(int row, int col) {
        return (row * (cols_ + 1) + col) * 4 + 2;
    }

    bool MotionVectorCallback::check_col(MMAL_BUFFER_HEADER_T *buffer, bool *searched, int row, int col, int height) {
        for (int i = row; i < row + height; i++) {
            if (!(searched[row * cols_ + col]) && buffer->data[buffer_pos(i, col)] > MOTION_THRESHOLD) {
                return true;
            }
        }
        return false;
    }

    bool MotionVectorCallback::check_row(MMAL_BUFFER_HEADER_T *buffer, bool *searched, int row, int col, int width) {
        for (int i = col; i < col + width; i++) {
            if (!(searched[row * cols_ + col]) && buffer->data[buffer_pos(row, i)] > MOTION_THRESHOLD) {
                return true;
            }
        }
        return false;
    }

    void MotionVectorCallback::callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
        if ((buffer->flags & MMAL_BUFFER_HEADER_FLAG_CODECSIDEINFO)) {
            int elements = rows_ * cols_;
            bool *searched = new bool[elements];
            for (int i = 0; i < elements; searched[i++] = false);
            for (int row = 0; row < rows_; row++) {
                for (int col = 0; col < cols_; col++) {
                    if (searched[row * cols_ + col]) {
                        break;
                    }
                    if (buffer->data[buffer_pos(row, col)] > MOTION_THRESHOLD) {
                        MotionRegion region = MotionRegion(row, col);

                        // search left hand side
                        while (check_col(buffer, searched, region.row, region.col, region.height) && region.grow_left());
                        // search right hand side
                        while (check_col(buffer, searched, region.row, region.col + region.width, region.height) && region.grow_right());
                        // search top side
                        while (check_row(buffer, searched, region.row, region.col, region.width) && region.grow_up());
                        // search bottom side
                        while (check_row(buffer, searched, region.row + region.height, region.col, region.width) && region.grow_down());

                        for (int i = region.row; i < region.row + region.height; i++) {
                            for (int j = region.col; j < region.col + region.width; j++) {
                                searched[i * cols_ + j] = true;
                            }
                        }

                        regions.push_back(region);
                        
                    }
                }
            }
            delete searched;
            new_vectors = true;
            vcos_log_error("MotionVectorCallback::callback(): Finished successfully");
        }
    }

    void MotionVectorCallback::post_process() {
        if (new_vectors) {
            new_vectors = false;
        }
    }
}
