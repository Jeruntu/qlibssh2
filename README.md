# qlibssh2

Qt wrapper for libssh2

## Introduction

I could not get libssh2 to work properly with QTcpSocket's. Than
I found the solution in the Qt ssh classes from project [daggy](https://github.com/synacker/daggy).
I decided to isolate the ssh2 related code and make it a small
library on it's own: ```qlibssh2```.

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
using namespace daggy;
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
environment with your compiler, Qt kit and cmake.

### Conan

First create the package in the local cache:

```powershell
PS git clone https://github.com/Jeruntu/qlibssh2.git
PS cd qlibssh2
PS md build; cd build
PS conan create . -s build_type=Release
```

To use the package in your application / library add this package
as a dependency:

```ini
[requires]
qlibssh2/0.1.0

[generators]
cmake_paths
cmake_find_package
```

```powershell
PS cd $YourAppDir
PS md build; cd build
PS conan install ..
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
git clone https://github.com/libssh2/libssh2.git
cd libssh2
md build; cd build
cmake .. -DBUILD_SHARED_LIBS=OFF
cmake --build .
```

The libssh2 cmake config package will be exported automatically
by the cmake build, and can be consumed with find_package. Now
build and install this library:

```powershell
PS git clone https://github.com/Jeruntu/qlibssh2.git
PS cd qlibssh2
PS md build; cd build
PS cmake .. -DCMAKE_PREFIX_PATH="$Env:QTDIR;-DCMAKE_INSTALL_PREFIX="$MyCmakePackageDir"
PS cmake --build .
PS cmake --build . --target install
```

```powershell
PS cd $YourAppDir
PS md build; cd build
PS cmake .. -DCMAKE_PREFIX_PATH="$Env:QTDIR;$MyCmakePackageDir"
PS cmake --build .
```
