message("Building technology tests")
add_executable(detect_blobs detect_blobs.cpp)
add_executable(detect_markers detect_markers.cpp)
add_executable(detect_contours detect_contours.cpp)
add_executable(detect_bounds detect_bounds.cpp)
target_link_libraries(detect_blobs ${OpenCV_LIBS} ${Boost_LIBRARIES} Threads::Threads mrtable tuio)
target_link_libraries(detect_markers ${OpenCV_LIBS}  ${Boost_LIBRARIES} Threads::Threads mrtable tuio)
target_link_libraries(detect_contours ${OpenCV_LIBS} ${Boost_LIBRARIES} Threads::Threads mrtable tuio)
target_link_libraries(detect_bounds ${OpenCV_LIBS} ${Boost_LIBRARIES} Threads::Threads mrtable tuio)

add_executable(omx_frames omx_frames.cpp)
target_link_libraries(omx_frames ${OpenCV_LIBS} ${Boost_LIBRARIES} Threads::Threads raspberrypi-omxcam )

add_executable(mmal_test mmal_test.cpp)
target_link_libraries(mmal_test ${MMAL_LIBRARIES} Threads::Threads)

add_executable(raspivid RaspiVid.c RaspiCLI.c RaspiPreview.c RaspiCamControl.c RaspiCLI.h RaspiPreview.h RaspiCamControl.h)
target_link_libraries(raspivid ${MMAL_LIBRARIES} Threads::Threads )
