#pragma once

namespace ParallelUtils {
    std::string GetOpenCLError(cl_int error);
    std::string GetDeviceInfo(cl_device_id device);
    std::string GetPlatformInfo(cl_platform_id platform);

    bool CreateProgram(cl_context context, cl_device_id device,
        const std::string& kernelName,
        const std::string& kernelSource,
        cl_program* program, cl_kernel* kernel);
}
