## Introduction
This is a model of a planar neural field that simulates evolution of activity rate of neurons implemented using the Amari equation.

This project is written in C++ and it uses CMake to generate platform-specific build files. Program uses OpenGL 3.3 or higher for rendering and ImGui library for the UI. Linear outlines are produced using the [Marching squares](https://en.wikipedia.org/wiki/Marching_squares) algorithm. Matrix algebra and marching squares use [OpenMP API](https://en.wikipedia.org/wiki/OpenMP) for paralleling calculations on CPU.

## Sceenshots
![Neural field simulation on Windows](images/NeuralFieldWin.png)

![Neural field simulation on Linux 1](images/NeuralFieldLinux1.png)

![Neural field simulation on Linux 2](images/NeuralFieldLinux2.png)

## Description of the Amari Neural Field
Planar neural field is modeled using an Amari model that uses the solution of the Cauchy problem for integro-differential equation:

![Amari equation](images/amariEquation.png)

* `u(x,t)` - activity function,
* `h` - equilibrium potential,
* `H(x)` - Heaviside function,
* `w` - weight function,
* `s` - external inhibition,
* `phi` - initial distribution of an electric potential at `t=0`,
* `x` - coordinate in the area `Omega`.

## Prerequisites
You need CMake to generate platform-specific makefiles or project files. This repository bundles most of the dependencies as git submodules, which includes:

* [glad](https://github.com/Dav1dde/glad) - OpenGL Function Loader.
* [glfw](https://github.com/glfw/glfw) - Windowing and Input.
* [glm](https://github.com/g-truc/glm) - OpenGL Mathematics.
* [plog](https://github.com/SergiusTheBest/plog) - Logging library.
* [imgui](https://github.com/ocornut/imgui) - UI library.

## Building for Linux

### Dependencies
The following instructions apply to:

* Ubuntu 20.04, 18.04, 16.04
* Debian 9 and higher

```
apt-get install \
    build-essential \
    cmake \
    xorg-dev \
    libgl1-mesa-dev \
    libfreetype6-dev
```

The following instructions apply to:

* Fedora 22 and higher

```
dnf install \
    cmake \
    gcc-c++ \
    mesa-libGL-devel \
    libXrandr-devel \
    libXinerama-devel \
    libXcursor-devel \
    libXi-devel \
    freetype-devel
```

* CentOS 7 and higher

```
yum install \
    cmake \
    gcc-c++ \
    mesa-libGL-devel \
    libXrandr-devel \
    libXinerama-devel \
    libXcursor-devel \
    libXi-devel \
    freetype-devel
```

### Cloning Repository
```
git clone --recursive https://github.com/Postrediori/NeuralField.git
cd NeuralField
```

The `--recursive` option automatically clones the required Git submodules too.

### Building Project
The program is built with the commands below. CMake requires the directory with the main project's `CMakeLists.txt` file as an argument. Then the CMake creates the build files for the GNU make which build an executable.

```
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
make install
```

### Running Project
After the successful build the binary `NeuralField` will end up in `<source dir>/bundle/NauralField` directory.

```
cd bundle/NeuralField
./NeuralField
```


All of the files required for an executable to run are stored in the `<PathToProject>/bundle/NeuralField`:

```
cd <PathToProject>/bundle
tree
.
└── NeuralField
    ├── NeuralField
    └── data
        ├── amari.conf
        ├── plane.frag
        ├── plane.vert
        └── ...

2 directories, 7 files
```

## Configuration
The parameters of the model can be adjusted in the `data/amari.conf` file:

```
# size = 128 | 256 | 512
size = 128

# h = 0..-0.3
h = -0.2

k = 0.05
K = 0.125
m = 0.025

# M = 0.05..0.07, 0.0625 - optimal
M = 0.065

# mode = wrap | reflect | mirror
mode = wrap
```

* `size` - Size of discrete neural field.
* `M` - Activation spread parameter. Smaller values stand for easily activated naural field,
larger values lead to smaller activity spread from the same activity initiator.
* `h` - Discrepancy parameter. Zero stands for no discrepancies and leads to symmetric model.
Smaller values lead to unsymmetric development of an activity.
* `mode` - Behavior on the neural field boundaries. `wrap` stands for the possibility of
boundary neurons to influence the opposite boundary. `reflect` stands for boundary as the line
of an active neurons.

h=0  | h=-0.15 | h=-0.3
---- | ------- | ------
![Neural Field With h=0](images/neural1.gif) | ![Neural Field With h=-0.15](images/neural2.gif) | ![Neural Field With h=-0.3](images/neural3.gif)


## Controls
* `F1` - Toggle fullscreen mode.
* `F2` - Show/hide help on the screen.
* `Space` or `RMB` - Clear the model.
* `LMB` - Initiate the activity in a pointunder cursor.
* `1..5` - Switch between output modes.
* `B` - Toggle blurring for the textured mode.

## Links
* S. Amari, [Dynamics of pattern formation in lateral-inhibition type neural filed](http://www.math.pitt.edu/~troy/sflood/amari.pdf), 1977.
* Konstantin Doubrovinski, Dynamics, [Stability and Bifurcation Phenomena in the Nonlocal Model of Cortical Activity](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.64.8688&rep=rep1&type=pdf), 2005.
* Dequan Jin, Dong Liang, Jigen Peng, [Existence and Properties of Stationary Solution of Dynamical Neural Field](http://gr.xjtu.edu.cn/c/document_library/get_file?folderId=29529&name=DLFE-2974.pdf), 2011.
* Stephen Coombes, Helmut Schmidt, Ingo Bojak, [Interface Dynamics in Planar Neural Field Models](http://www.mathematical-neuroscience.com/content/2/1/9), 2012.
* Luca Salasnich, [Power Spectrum and Diffusion of the Amari Neural Field](https://doi.org/10.3390/sym11020134), 2019.

## TODO
* [  ] Build instructions for Windows.
* [  ] Build instructions for macOS X: add OpenMP setup or make more universal version without OpenMP.
* [  ] Matrix algebra using hardware acceleration (OpenCL, OpenGL Compute Shaders, texture rendering&framebuffers, shader feedback, etc.)
* [  ] Export results to images and videos.
* [  ] Expand description of the model.
