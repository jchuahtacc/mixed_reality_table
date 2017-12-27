add_executable(still-jpeg jpeg/jpeg.c)
add_executable(still-rgb rgb/rgb.c)
add_executable(still-yuv yuv/yuv.c)

set_target_properties(still-jpeg PROPERTIES COMPILE_FLAGS ${RPI_OMXCAM_C_FLAGS})
set_target_properties(still-rgb PROPERTIES COMPILE_FLAGS ${RPI_OMXCAM_C_FLAGS})
set_target_properties(still-yuv PROPERTIES COMPILE_FLAGS ${RPI_OMXCAM_C_FLAGS})

target_link_libraries(still-jpeg ${BROADCOM_LIBRARIES} raspberrypi-omxcam )
target_link_libraries(still-rgb ${BROADCOM_LIBRARIES} raspberrypi-omxcam )
target_link_libraries(still-yuv ${BROADCOM_LIBRARIES} raspberrypi-omxcam )

