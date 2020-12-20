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
)

target_include_directories(qlibssh2
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(qlibssh2
    PUBLIC
        Qt5::Core
        Qt5::Network
        Libssh2::libssh2
)

target_compile_features(qlibssh2 PUBLIC cxx_std_17)

include(GNUInstallDirs)
set(PACKAGE_NAME ${CMAKE_PROJECT_NAME})
install(TARGETS qlibssh2
    EXPORT ${PACKAGE_NAME}Config
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
install(DIRECTORY src/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PACKAGE_NAME}
    )
install(EXPORT ${PACKAGE_NAME}Config
    NAMESPACE QLIBSSH2::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PACKAGE_NAME}
    )
