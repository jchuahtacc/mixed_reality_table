#include "rpi_motioncam/callbacks/MotionVectorCallback.h"
#include <opencv2/core.hpp>
#include <iostream>

namespace rpi_motioncam {

    MotionVectorCallback::MotionVectorCallback(RPIMOTIONCAM_OPTION_S options) : cols_(options.resizer_width / 16), rows_(options.resizer_height / 16), options_(options), width_scale(options.width / cols_), height_scale(options.height / rows_) {
        int elements = rows_ * cols_;
        searched = new char[elements];
        lastBuffer = new char[elements];
        if (options.preview) {
            vector_preview = shared_ptr< VectorPreview >( new VectorPreview( options ) );
            frame_preview = shared_ptr< FramePreview >( new FramePreview( options ) );
        } else {
            vector_preview = nullptr;
            frame_preview = nullptr;
        }
    }

    MotionVectorCallback::~MotionVectorCallback() {
        delete lastBuffer;
        delete searched;
    }

    int MotionVectorCallback::buffer_pos(int row, int col) {
        return (row * (cols_ + 1) + col) * 4 + 2;
    }

    bool MotionVectorCallback::grow_up(shared_ptr< MotionRegion > region) {
        if (region->row > 0) {
            region->row = region->row - 1;
            region->height = region->height + 1;
            return true;
        }
        return false;
    }

    bool MotionVectorCallback::grow_down(shared_ptr< MotionRegion > region) {
        if (region->row + region->height < rows_) {
            region->height = region->height + 1;
            return true;
        }
        return false;
    }

    bool MotionVectorCallback::grow_left(shared_ptr< MotionRegion > region) {
        if (region->col > 0) {
            region->col = region->col - 1;
            region->width = region->width + 1;
            return true;
        }
        return false;
    }

    bool MotionVectorCallback::grow_right(shared_ptr< MotionRegion > region) {
        if (region->col + region->width < cols_) {
            region->width = region->width + 1;
            return true;
        }
        return false;
    }

    bool MotionVectorCallback::check_left(MMAL_BUFFER_HEADER_T *buffer, char *searched, shared_ptr< MotionRegion > region) {
        for (int row = region->row; row < region->row + region->height; row++) {
            if (!(searched[row * cols_ + region->col]) && buffer->data[buffer_pos(row, region->col)] > options_.motion_threshold) {
                return grow_left(region);
            }
        }
        return false;
    }

    bool MotionVectorCallback::check_right(MMAL_BUFFER_HEADER_T *buffer, char *searched, shared_ptr< MotionRegion > region) {
        for (int row = region->row; row < region->row + region->height; row++) {
            if (!(searched[row * cols_ + region->col]) && buffer->data[buffer_pos(row, region->col + region->width - 1)] > options_.motion_threshold) {
                return grow_right(region);
            }
        }
        return false;
    }

    bool MotionVectorCallback::check_top(MMAL_BUFFER_HEADER_T *buffer, char *searched, shared_ptr< MotionRegion > region) {
        for (int col = region->col; col < region->col + region->width; col++) {
            if (!(searched[region->row * cols_ + col]) && buffer->data[buffer_pos(region->row, col)] > options_.motion_threshold) {
                return grow_up(region);
            }
        }
        return false;
    }

    bool MotionVectorCallback::check_bottom(MMAL_BUFFER_HEADER_T *buffer, char *searched, shared_ptr< MotionRegion > region) {
        for (int col = region->col; col < region->col + region->width; col++) {
            if (!(searched[region->row * cols_ + col]) && buffer->data[buffer_pos(region->row + region->height - 1, col)] > options_.motion_threshold) {
                return grow_down(region);
            }
        }
        return false;
    }

    cv::Rect MotionVectorCallback::calculate_roi(shared_ptr< MotionRegion > region) {
        return cv::Rect(region->col * width_scale, region->row * height_scale, region->width * width_scale, region->height * height_scale);
    }

    void MotionVectorCallback::grow_region(MMAL_BUFFER_HEADER_T *buffer, char *searched, shared_ptr< MotionRegion > region) {
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
            vector< shared_ptr< MotionRegion > > regions;
            for (int i = 0; i < rows_ * cols_; searched[i++] = false);
            for (int row = 0; row < rows_; row++) {
                for (int col = 0; col < cols_; col++) {
                    lastBuffer[row * cols_ + col] = buffer->data[buffer_pos(row, col)];
                }
            }
            for (int row = 0; row < rows_; row++) {
                for (int col = 0; col < cols_; col++) {
                    if (searched[row * cols_ + col]) {
                        break;
                    }
                    if (buffer->data[buffer_pos(row, col)] > options_.motion_threshold) {
                        auto region = shared_ptr< MotionRegion >(new MotionRegion());
                        region->row = row;
                        region->col = col;
                        region->width = 1;
                        region->height = 1;
                        grow_up(region);
                        grow_down(region);
                        grow_left(region);
                        grow_right(region);
                        grow_region(buffer, searched, region);

                        region->allocate(calculate_roi(region));

                        for (int i = region->row; i < region->row + region->height; i++) {
                            for (int j = region->col; j < region->col + region->width; j++) {
                                searched[i * cols_ + j] = true;
                            }
                        }

                        regions.push_back(region);
                        
                    }
                }
            }
            if (regions.size()) {
                lastFrame = MotionFrame( regions );
                MotionData::stage_frame( regions );
            } else {
                lastFrame.regions.clear();
            }
        }
    }

    void MotionVectorCallback::post_process() {
        if (vector_preview) {
            vector_preview->draw( lastBuffer );
        }
        if (frame_preview) {
            frame_preview->draw( lastFrame );
        }
    }
}
