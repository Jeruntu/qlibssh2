# qlibssh2

![CI](https://github.com/jeruntu/qlibssh2/workflows/CI/badge.svg)

Qt wrapper for libssh2

## Introduction

I could not get libssh2 to work properly with QTcpSocket's. Than
I found the solution in the Qt ssh classes from project
[daggy](https://github.com/synacker/daggy). I decided to isolate
the ssh2 related code and make it a small library on it's own:
```qlibssh2```.

## How to use

### CMake

In the cmake project of your application:

```cmake
find_package(QLibssh2 REQUIRED CONFIG)

add_executable(example
    main.cpp
    # ...
)

target_link_libraries(example QLIBSSH2::qlibssh2)
```

The find_package will check that all needed dependencies of this
library are installed. These dependencies are libssh2 and Qt5Core,
Qt5Network. The libssh2 library has dependencies on OpenSSL and zlib.

### API

```cpp
using namespace qlibssh2;
int main() {
    Ssh2Client client{{}};
    # TODO add api logic here
}
```

## Build

There are two ways to build this library. The easiest way is with
[Conan](https://conan.io/), but the cmake project has been written
in a way that Conan is just optional and all the dependencies can
also be installed manually. For both methods, first prepare a build
environment with your compiler and cmake.

### Conan

Since this library is a Qt wrapper around libssh2, it is likely you
have Qt pre-installed for your application. To build this library with
your pre-installed Qt version you will have to define the QTDIR environment
variable and set the option `local_qt_version`. If this option is not defined,
conan will build Qt from sources.

#### Export package recipe to the local cache

```powershell
PS git clone https://github.com/Jeruntu/qlibssh2.git
PS cd qlibssh2
PS conan export .
```

#### Add this library as a dependency

To use the package in your application / library, add this package as a dependency
in conanfile.txt or conanfile.py:

```ini
[requires]
qlibssh2/0.1.0

[generators]
cmake_paths
cmake_find_package
```

#### Install dependencies for your project

```powershell
PS cd $YourAppDir
PS md build; cd build
```

#### (1) Local pre-installed Qt

```powershell
PS $Env:QTDIR='C:\Qt\5.15.1\msvc2019_64'
PS conan install .. -o local_qt_version='5.15.1' --build=missing
```

To make this fixed, add it to the conanfile.txt and omit the option
from the command line:

```ini
[options]
qlibssh2:local_qt_version='5.15.1'
```

#### (2) Or build Qt from sources (takes a while...)

```powershell
PS conan remote add bincrafters 'https://api.bintray.com/conan/bincrafters/public-conan'
PS conan install .. --build=missing
```

To change the Qt version override it:

```ini
[requires]
qt/5.15.2
```

#### Build project

Now that all dependencies are installed by ```Conan```, the project is
ready to be build. Pass the generated ```conan_paths.cmake``` file on
to the main cmake file of your project:

```powershell
PS cmake .. "-DCMAKE_PROJECT_$(APP_PROJECT_NAME)_INCLUDE=$pwd/conan_paths.cmake"
PS cmake --build .
```

### Manually

For libssh2 OpenSSL can be used as the crypto lib. The easiest way
is to install it with [chocolatey](https://chocolatey.org/).

```powershell
PS choco install openssl
```

Make sure the OpenSSL dll's are in the PATH. Build libssh2:

```powershell
PS git clone https://github.com/libssh2/libssh2.git
PS cd libssh2
PS build; cd build
PS cmake .. -DBUILD_SHARED_LIBS=OFF
PS cmake --build .
```

The libssh2 cmake config package will be exported automatically
by the cmake build, and can be consumed with find_package. Now
build and install this library:

```powershell
PS git clone https://github.com/Jeruntu/qlibssh2.git
PS cd qlibssh2
PS md build; cd build
PS cmake .. -DCMAKE_PREFIX_PATH="$Env:QTDIR" -DCMAKE_INSTALL_PREFIX="$MyCmakePackageDir"
PS cmake --build .
PS cmake --build . --target install
```

```powershell
PS cd $YourAppDir
PS md build; cd build
PS cmake .. -DCMAKE_PREFIX_PATH="$Env:QTDIR;$MyCmakePackageDir"
PS cmake --build .
```
