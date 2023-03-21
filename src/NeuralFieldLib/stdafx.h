#pragma once

#include <plog/Log.h>

#include <cmath>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <string>

#ifdef USE_OPENCL
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
#endif
