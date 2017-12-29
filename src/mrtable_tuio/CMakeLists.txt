set(CMAKE_CXX_STANDARD 11)
include_directories("${PROJECT_SOURCE_DIR}/lib")
add_library(mrtable_tuio StdErrSender.cpp)
