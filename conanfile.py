from conans import ConanFile, CMake
import os

class Qlibssh2Conan(ConanFile):
    name = "qlibssh2"
    version = "0.1.0"
    license = "MIT"
    url = ""
    author = "Jeroen Oomkes (jeruntu@gmail.com)"
    description = "Qt wrapper for libssh2"
    topics = ("ssh", "ssh2", "qt")
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "local_qt_version": "ANY",
    }
    default_options = {
        "local_qt_version": None,
    }
    generators = "cmake_paths", "cmake_find_package"
    requires = [
        "libssh2/[>=1.9.0]",
        "openssl/1.1.1g",
    ]
    exports_sources = [
        "src/*",
        "CMakeLists.txt",
        "cmake/QLibssh2Config.cmake.in",
    ]

    def config_options(self):
        pass

    def requirements(self):
        if self.options.local_qt_version:
            if os.getenv('QTDIR') is None:
                raise Exception("To use local pre installed Qt, QTDIR env variable has to be defined")
        else:
            self.requires("qt/[>=5.15.0 <6.0.0]@bincrafters/stable")

    def configure(self):
        self.options["libssh2"].shared = False

        if not self.options.local_qt_version:
            self.options["qt"].shared = True
            self.options["qt"].commercial = False

            self.options["qt"].opengl = "no"
            self.options["qt"].openssl = False
            self.options["qt"].with_vulkan = False
            self.options["qt"].with_pcre2 = False
            self.options["qt"].with_glib = False
            self.options["qt"].with_freetype = False
            self.options["qt"].with_fontconfig = False
            self.options["qt"].with_harfbuzz = False
            self.options["qt"].with_libjpeg = False
            self.options["qt"].with_libpng = False
            self.options["qt"].with_sqlite3 = False
            self.options["qt"].with_mysql = False
            self.options["qt"].with_pq = False
            self.options["qt"].with_odbc = False
            self.options["qt"].with_sdl2 = False
            self.options["qt"].with_libalsa = False
            self.options["qt"].with_openal = False
            self.options["qt"].with_zstd = False
            self.options["qt"].with_pq = False
            self.options["qt"].GUI = False
            self.options["qt"].widgets = False

    def build(self):
        defs = None
        if self.options.local_qt_version:
            qtdir = os.getenv('QTDIR').replace('\\', '/')
            defs = { 'CMAKE_PREFIX_PATH': qtdir }

        cmake = CMake(self)
        cmake.configure(defs=defs)
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["qlibssh2"]
