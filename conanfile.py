from conans import ConanFile, CMake


class Qlibssh2Conan(ConanFile):
    name = "qlibssh2"
    version = "0.1.0"
    license = "MIT"
    url = ""
    author = "Jeroen Oomkes (jeruntu@gmail.com)"
    description = "Qt wrapper for libssh2"
    topics = ("ssh", "ssh2", "qt")
    settings = "os", "compiler", "build_type", "arch"
    options = {}
    default_options = {}
    generators = "cmake_paths", "cmake_find_package"
    requires = [
        "libssh2/[>=1.9.0]",
        "openssl/1.1.1g"
        # "qt/5.15.2@bincrafters/stable",
    ]
    exports_sources = [
        "src/*",
        "CMakeLists.txt",
        "cmake/QLibssh2Config.cmake.in"
    ]

    def config_options(self):
        pass

    def configure(self):
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

        self.options["libssh2"].shared = False

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["qlibssh2"]
