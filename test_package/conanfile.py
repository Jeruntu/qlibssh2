import os

from conans import ConanFile, CMake, tools


class Qlibssh2TestConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "local_qt_version": "ANY",
    }
    default_options = {
        "local_qt_version": None,
    }
    generators = "cmake_paths", "cmake_find_package"

    def build(self):
        cwd = os.getcwd().replace('\\', '/')
        defs = { "CMAKE_PROJECT_PackageTest_INCLUDE": "{}/conan_paths.cmake".format(cwd) }
        if self.options.local_qt_version:
            qtdir = os.getenv('QTDIR').replace('\\', '/')
            defs['CMAKE_PREFIX_PATH'] = qtdir

        cmake = CMake(self)
        cmake.configure(defs=defs)
        cmake.build()

    def test(self):
        if not tools.cross_building(self):
            self.run(".%sbin%sexample" % (os.sep, os.sep))
