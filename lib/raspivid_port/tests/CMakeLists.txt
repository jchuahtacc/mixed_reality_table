message(STATUS "Building rpi_motioncam test programs")
include_directories("${BROADCOM_INCLUDE_DIRS}")
include_directories("${MMAL_INCLUDE_DIRS}")
include_directories("${RPI_MOTIONCAM_INCLUDE_DIRS}")
add_executable(rpi_motioncam_preview RPiMotionCamPreview.cpp)
target_link_libraries(rpi_motioncam_preview rpi_motioncam)

