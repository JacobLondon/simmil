# simmil
[SDL2-2.0.10](https://www.libsdl.org/)
for Linux and Visual Studio 2019

# Examples
![Screenshot](https://user-images.githubusercontent.com/17059471/126886608-7d2937fc-3b30-45af-bb0a-ccef7f86c52b.png)

Click + drag to move, space to reset position, scroll wheel to zoom in and out. Rendered area is the red box.

# Building & Dependencies
## Linux
```
$ sudo apt-get install libsdl2-dev libsdl2-image-dev build-essential
$ make
```
## Visual Studio 2019
Move all DLLs under `/lib/x86/` or `/lib/x64/` into the root directory depending on the build configuration.

| All Config Properties | Option | Spec |
|-----------------------|--------|------|
| General | C++ Language Standard | ISO C++17 Standard (std:c++17) |
| C/C++ | Additional Include Directories | $(ProjectDir)/include |
| C/C++ | Code Generation: Basic Runtime Checks | Default |
| C/C++ | Optimization: Optimization | Maximum Optimization (Favor Speed) (/O2) |
| C/C++ | Optimization: Favor Size Or Speed | Favor fast code (/Ot) |
| C/C++ | Language: C++ Language Standard | ISO C++17 Standard (/std:c++17) |
| Linker | Additional Dependencies: (Active(Win32) or Win32) | lib/x86/*.lib |
| Linker | Additional Dependencies: (x64) | lib/x64/*.lib |
