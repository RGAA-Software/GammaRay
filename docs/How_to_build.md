### How to build
#### 1. Clone the repo
```c++
    git clone --recursive https://github.com/RGAA-Software/GammaRay.git
```

#### 2. Install dependencies by VCPKG in 3rdparty.
- 2.1 Change to VCPKG folder
```c++
    cd {your-project-folder}/deps/tc_3rdparty/vcpkg
```
- 2.2 Install vcpkg.exe
```c++
    .\bootstrap-vcpkg.bat 
```
- 2.3 Install dependencies
```c++
    .\vcpkg.exe install gflags:x64-windows
    .\vcpkg.exe install sqlite3:x64-windows
    .\vcpkg.exe install detours:x64-windows
    .\vcpkg.exe install gtest:x64-windows
    .\vcpkg.exe install libvpx:x64-windows
    .\vcpkg.exe install opus:x64-windows
    .\vcpkg.exe install fftw3:x64-windows
    .\vcpkg.exe install easyhook:x64-windows
    .\vcpkg.exe install glm:x64-windows
    .\vcpkg.exe install sdl2:x64-windows
    .\vcpkg.exe install jemalloc:x64-windows
	.\vcpkg.exe install cpr:x64-windows
	.\vcpkg.exe install mongo-cxx-driver:x64-windows
	.\vcpkg.exe install drogon:x64-windows
    .\vcpkg.exe install breakpad:x64-windows-static
```

- 2.4 You can open the project by Visual Studio 2022 or Clion, solve the problems{mostly you need to do :), b/c it's a cpp project, you know it...} and then compile the project.
