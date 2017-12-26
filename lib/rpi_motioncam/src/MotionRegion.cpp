#include "rpi_motioncam/MotionRegion.h"

namespace rpi_motioncam {
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
}
