message("Building mrtable_process tests")
include_directories("../")
add_executable(test_process test_process.cpp)
target_link_libraries(test_process mrtable_process)
