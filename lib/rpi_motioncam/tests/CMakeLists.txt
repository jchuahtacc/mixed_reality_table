message(STATUS "Building rpi_motioncam test programs")
include_directories("${RPI_MOTIONCAM_INCLUDE_DIRS}")
add_executable(rpi_motioncam_preview RPiMotionCamPreview.cpp)
add_executable(test_rectangles RectangleDetect.cpp)
target_link_libraries(rpi_motioncam_preview rpi_motioncam)
target_link_libraries(test_rectangles rpi_motioncam)

