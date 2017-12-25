cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 11)
set(THREADS_PREFER_PTHREAD_FLAG ON)
message(STATUS "Building rpi_motioncam")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package( MMAL REQUIRED )
find_package( Broadcom REQUIRED )
find_package( OpenCV REQUIRED )
find_package( TBB REQUIRED )

add_definitions(-DTBB_USE_GCC_BUILTINS=1)
add_definitions(-D__TBB_64BIT_ATOMICS=0)

set(BUILD_RPIMOTIONCAM_TESTS FALSE CACHE PATH "Build RPiMotionCam Test Programs")

include_directories("${BROADCOM_INCLUDE_DIRS}")
include_directories("${MMAL_INCLUDE_DIRS}")

set(RPI_MOTIONCAM_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE PATH "rpi_motioncam header files")

include_directories("${RPI_MOTIONCAM_INCLUDE_DIRS}")

add_library(rpi_motioncam ./src/RaspiVid.cpp ./src/components/RaspiRenderer.cpp ./src/RaspiCamControl.cpp ./src/components/RaspiComponent.cpp ./src/components/RaspiNullsink.cpp ./src/components/RaspiEncoder.cpp ./src/RaspiPort.cpp ./src/MotionVectorCallback.cpp ./src/components/RaspiSplitter.cpp ./src/components/RaspiResize.cpp ./src/components/RaspiCamera.cpp ./src/components/RaspiOverlayRenderer.cpp ./src/MotionVectorPreviewCallback.cpp ./src/RawOutputCallback.cpp ./src/MotionData.cpp
    ./src/MotionRegion.cpp)
target_link_libraries(rpi_motioncam ${OpenCV_LIBS} ${MMAL_LIBRARIES} pthread tbb)

if (BUILD_RPIMOTIONCAM_TESTS)
    add_subdirectory(tests)
endif(BUILD_RPIMOTIONCAM_TESTS)
