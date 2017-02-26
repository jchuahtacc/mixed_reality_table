message("Building fiducial generators")
add_executable(generate_markers generate_markers.cpp)
add_executable(generate_charuco generate_charuco.cpp)

target_link_libraries(generate_markers ${OpenCV_LIBS} ${Boost_LIBRARIES} mrtable tuio)
target_link_libraries(generate_charuco ${OpenCV_LIBS} ${Boost_LIBRARIES} Threads::Threads mrtable tuio)
