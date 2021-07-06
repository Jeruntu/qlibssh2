cmake_minimum_required(VERSION 3.16)

project(QLibssh2 LANGUAGES CXX)

find_package(Qt5 REQUIRED
    Core
    Network
)

find_package(Libssh2 REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON) # This is to find generated *.moc and *.h files in build dir
set(CMAKE_AUTOMOC ON)

add_library(qlibssh2 STATIC
    src/Ssh2Channel.cpp
    src/Ssh2Client.cpp
    src/Ssh2Debug.cpp
    src/Ssh2Process.cpp
    src/Ssh2Types.cpp
    src/Ssh2Scp.cpp
    src/Ssh2LocalPortForwarding.cpp

    # For automoc
    src/include/Ssh2Client.h
    src/include/Ssh2LocalPortForwarding.h
    src/include/Ssh2Process.h
    src/include/Ssh2Scp.h
    src/include/Ssh2Types.h
)

target_include_directories(qlibssh2
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        src
)

target_link_libraries(qlibssh2
    PUBLIC
        Qt5::Core
        Qt5::Network
        Libssh2::libssh2
)

target_compile_features(qlibssh2 PUBLIC cxx_std_17)

if(NOT ${QLIBSSH2_SKIP_INSTALL_PACKAGE})
    # --- Create installed package
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)
    set(PACKAGE_NAME ${CMAKE_PROJECT_NAME})

    set(TARGETS_FILE ${CMAKE_INSTALL_LIBDIR}/cmake/${PACKAGE_NAME}/${PACKAGE_NAME}Targets.cmake)
    configure_package_config_file(
        cmake/${PACKAGE_NAME}Config.cmake.in
        ${CMAKE_BINARY_DIR}/${PACKAGE_NAME}Config.cmake
        INSTALL_DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/${PACKAGE_NAME}
        PATH_VARS
        TARGETS_FILE
        )

    get_filename_component(TARGETS_FILE_WLE ${TARGETS_FILE} NAME_WLE)
    install(TARGETS qlibssh2
        EXPORT ${TARGETS_FILE_WLE}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    install(EXPORT ${TARGETS_FILE_WLE}
        NAMESPACE QLIBSSH2::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PACKAGE_NAME}
        )
    install(
        FILES
        ${CMAKE_BINARY_DIR}/${PACKAGE_NAME}Config.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PACKAGE_NAME}
        )
    install(DIRECTORY src/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PACKAGE_NAME}
        )
endif()
