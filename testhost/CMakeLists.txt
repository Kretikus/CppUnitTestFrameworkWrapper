add_executable(testhost main.cpp)

target_link_libraries(testhost dl)
target_link_libraries(testhost Boost::system Boost::filesystem)
