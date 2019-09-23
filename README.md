# What is it?

# Building
## External dependencies
* [C++ REST SDK](https://github.com/microsoft/cpprestsdk)
* [CUDA Toolkit](https://developer.nvidia.com/cuda-downloads)
* (Windows) [vcpkg](https://github.com/Microsoft/vcpkg)

## Libraries version used to compile and run test builds:
* C++ REST SDK - ???
* CUDA Toolkit - 10.1

## Building
### Set up vcpkg
```
PS> git clone https://github.com/Microsoft/vcpkg.git
PS> cd vcpkg
PS> .\bootstrap-vcpkg.bat
PS> .\vcpkg integrate install
PS> .\vcpkg install cpprestsdk cpprestsdk:x64-windows
```

### Build the project
```
$ mkdir build
$ cd build
PS> (Windows) cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_TOOLCHAIN_FILE="<vcpkg_location>/vcpkg/scripts/buildsystems/vcpkg.cmake" ..
$ (Linux) cmake ???
```