# What is it?
A C++ applicaton simulating heat distiribution in a 2D body. It acts as a simple webserver responding to queries with image depicting computed result.

# External dependencies
* [C++ REST SDK](https://github.com/microsoft/cpprestsdk) (tested with v2.10.14-1)
* [OpenCV](https://opencv.org/) (tested with v4.1.1)
* [CUDA Toolkit](https://developer.nvidia.com/cuda-downloads) (tested with v10.1)
* [Boost](https://www.boost.org/) (tested with v1.70.0)
* [CMake](https://cmake.org/download/) (tested with v3.15.5)
* [vcpkg](https://github.com/Microsoft/vcpkg)
* [OneAgent C++ SDK](https://github.com/Dynatrace/OneAgent-SDK-for-C) (included as a git submodule)

# Building

## Set up vcpkg
Windows:
```
PS> git clone https://github.com/Microsoft/vcpkg.git
PS> cd vcpkg
PS> .\bootstrap-vcpkg.bat
PS> .\vcpkg integrate install
PS> .\vcpkg install cpprestsdk:x64-windows opencv:x64-windows boost-program-options:x64-windows
```
Linux:
```
$ git clone https://github.com/Microsoft/vcpkg.git
$ cd vcpkg
$ ./bootstrap-vcpkg.sh
$ ./vcpkg integrate install
$ ./vcpkg install cpprestsdk:x64-linux opencv:x64-linux boost-program-options:x64-linux
$ sudo apt-get install libgdcm-tools python-vtkgdcm libvtkgdcm2.6 libvtkgdcm-java
```

## Build the project
```
$ mkdir build
$ cd build
(Windows) PS> cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_TOOLCHAIN_FILE="<vcpkg_directory>/scripts/buildsystems/vcpkg.cmake" ..
(Linux) $ cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE="<vcpkg_directory>/scripts/buildsystems/vcpkg.cmake" ..
```
