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

include_directories("${BROADCOM_INCLUDE_DIRS}")
include_directories("${MMAL_INCLUDE_DIRS}")

set(BUILD_RPIMOTIONCAM_TESTS FALSE CACHE PATH "Build RPiMotionCam Test Programs")

include_directories("../")

add_library(rpi_motioncam RPiMotionCam.cpp components/RaspiRenderer.cpp RaspiCamControl.cpp components/RaspiComponent.cpp components/RaspiNullsink.cpp components/RaspiEncoder.cpp RaspiPort.cpp callbacks/MotionVectorCallback.cpp components/RaspiSplitter.cpp components/RaspiResize.cpp components/RaspiCamera.cpp components/RaspiOverlayRenderer.cpp callbacks/MotionVectorPreviewCallback.cpp callbacks/RawOutputCallback.cpp callbacks/MotionData.cpp callbacks/MotionRegion.cpp callbacks/MotionFrame.cpp)
target_link_libraries(rpi_motioncam ${OpenCV_LIBS} ${MMAL_LIBRARIES} pthread tbb)

if(BUILD_RPIMOTIONCAM_TESTS)
    add_subdirectory(tests)
endif(BUILD_RPIMOTIONCAM_TESTS)
