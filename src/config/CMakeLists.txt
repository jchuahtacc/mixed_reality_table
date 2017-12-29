set(CMAKE_CXX_STANDARD 11)
add_library(mrtable_config config.cpp ContourParams.cpp DetectBounds.cpp parsers.cpp ServerConfig.cpp)
target_link_libraries(mrtable_config ${OpenCV_LIBS})
