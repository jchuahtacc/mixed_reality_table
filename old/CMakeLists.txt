cmake_minimum_required(VERSION 3.1)
# Project config
set(CMAKE_CXX_STANDARD 11)
set(THREADS_PREFER_PTHREAD_FLAG ON)
project(mixed_reality_table)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Required packages
find_package( OpenCV REQUIRED )
find_package( Boost REQUIRED system )
find_package( Threads REQUIRED )
find_package( Broadcom REQUIRED )
find_package( MMAL REQUIRED )

# Optional executables
set(BUILD_TUIO_SIMULATORS FALSE CACHE PATH "Build TUIO Test programs")
set(BUILD_OMX_EXAMPLES FALSE CACHE PATH "Build raspberrypi-omxcam example programs" )
set(BUILD_TECH_TESTS FALSE CACHE PATH "Build mrtable technology tests")
set(BUILD_GENERATORS FALSE CACHE PATH "Build mrtable fiducial generator")

# CFlags

#set(CMAKE_CXX_FLAGS "-DSTANDALONE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DTARGET_POSIX -D_LINUX -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -U_FORTIFY_SOURCE -DHAVE_LIBOPENMAX=2 -DOMX -DOMX_SKIP64BIT -DUSE_EXTERNAL_OMX -DHAVE_LIBBCM_HOST -DUSE_EXTERNAL_LIBBCM_HOST -DUSE_VCHIQ_ARM -fPIC -ftree-vectorize -pipe -g -O3 -fvisibility=hidden -mcpu=cortex-a53 -mfpu=neon-fp-armv8 -funsafe-math-optimizations -mfloat-abi=hard")

# Add main libraries
add_subdirectory(lib)
include_directories("${PROJECT_SOURCE_DIR}/lib")
include_directories("${BROADCOM_INCLUDE_DIRS}")
include_directories("${MMAL_INCLUDE_DIRS}")

# Add main executables
add_executable(calibrate_camera src/calibrate_camera.cpp)
add_executable(server src/server.cpp)
target_link_libraries(calibrate_camera ${OpenCV_LIBS} ${Boost_LIBRARIES} Threads::Threads  mrtable tuio)
target_link_libraries(server ${OpenCV_LIBS} ${Boost_LIBRARIES} Threads::Threads mrtable tuio )

if(BUILD_TECH_TESTS)
    add_subdirectory(tech_tests)
endif(BUILD_TECH_TESTS)

if(BUILD_GENERATORS)
    add_subdirectory(generators)
endif(BUILD_GENERATORS)

if(BUILD_TUIO_SIMULATORS)
    add_subdirectory(tuio_simulators)
endif(BUILD_TUIO_SIMULATORS)
