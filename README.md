# Amari Model Plane Problem

## Description

3D plane simulation of neural system using the Amari Model.

## Environment Setup

### Fedora

```
sudo dnf install -y \
    cmake \
    freeglut-devel \
    glew-devel \
    glm-devel \
    freetype-devel
```

### Ubuntu 16.04

```
sudo apt-get install -y \
    cmake \
    freeglut3-dev \
    libglew-dev \
    libglm-dev \
    libfreetype6-dev
```

## Building

```
mkdir build && cd build
cmake ..
make -j4
```

## Running

```
cd build/AmariModel
./AmariModel
```

## References

* Konstantin Doubrovinski, Dynamics, Stability and Bifurcation Phenomena in the Nonlocal Model of Cortical Activity, 2005.
* Dequan Jin, Dong Liang, Jigen Peng, Existence and Properties of Stationary Solution of Dynamical Neural Field, 2011.
* Stephen Coombes, Helmut Schmidt, Ingo Bojak, Interface Dynamics in Planar Neural Field Models, 2012.

