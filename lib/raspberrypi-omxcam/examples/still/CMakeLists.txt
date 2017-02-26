add_executable(still-jpeg jpeg/jpeg.c)
add_executable(still-rgb rgb/rgb.c)
add_executable(still-yuv yuv/yuv.c)

target_link_libraries(still-jpeg ${BROADCOM_LIBRARIES} raspberrypi-omxcam )
target_link_libraries(still-rgb ${BROADCOM_LIBRARIES} raspberrypi-omxcam )
target_link_libraries(still-yuv ${BROADCOM_LIBRARIES} raspberrypi-omxcam )

