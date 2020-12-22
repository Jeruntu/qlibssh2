cmake_minimum_required(VERSION 3.16)

project(PackageTest CXX)

find_package(QLibssh2 REQUIRED CONFIG)

add_executable(example example.cpp)

target_link_libraries(example QLIBSSH2::qlibssh2)

set_target_properties(example
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
)

# CTest is a testing tool that can be used to test your project.
# enable_testing()
# add_test(NAME example
#          WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
#          COMMAND example)
