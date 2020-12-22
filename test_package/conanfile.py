import os

from conans import ConanFile, CMake, tools


class Qlibssh2TestConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_paths"

    def build(self):
        cmake = CMake(self)
        cwd = os.getcwd().replace('\\', '/')
        cmake.configure(defs={ "CMAKE_PROJECT_PackageTest_INCLUDE": "{}/conan_paths.cmake".format(cwd)} )
        cmake.build()

    def test(self):
        if not tools.cross_building(self):
            self.run(".%sbin%sexample" % (os.sep, os.sep))
