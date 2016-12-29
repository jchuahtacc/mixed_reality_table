include_directories("${PROJECT_SOURCE_DIR}/lib")
add_library(mrtable mrtable.cpp)
target_link_libraries(mrtable tuio)
