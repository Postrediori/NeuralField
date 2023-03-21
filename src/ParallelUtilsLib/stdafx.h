#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <sstream>

#include <plog/Log.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
