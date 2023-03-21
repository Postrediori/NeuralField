#include "stdafx.h"
#include "ParallelUtils.h"

static std::string GetDeviceInfoString(cl_device_id device, cl_device_info paramName) {
    cl_int status;
    size_t retSize = 0;
    status = clGetDeviceInfo(device, paramName, 0, nullptr, &retSize);
    if (status != CL_SUCCESS || retSize == 0) {
        return {};
    }

    std::vector<char> buf(retSize);
    status = clGetDeviceInfo(device, paramName, sizeof(buf[0]) * buf.size(), buf.data(), nullptr);
    if (status != CL_SUCCESS) {
        return {};
    }

    std::string str(buf.begin(), buf.end());

    // Use .c_str() to return string null-character at the end.
    // Null-terminated string will break the stringstream
    return str;
}

static std::string GetPlatformInfoString(cl_platform_id platform, cl_platform_info paramName) {
    cl_int status;
    size_t retSize = 0;
    status = clGetPlatformInfo(platform, paramName, 0, nullptr, &retSize);
    if (status != CL_SUCCESS || retSize == 0) {
        return {};
    }

    std::vector<char> buf(retSize);
    status = clGetPlatformInfo(platform, paramName, sizeof(buf[0]) * buf.size(), buf.data(), nullptr);
    if (status != CL_SUCCESS) {
        return {};
    }

    std::string str(buf.begin(), buf.end());

    // Use .c_str() to return string null-character at the end.
    // Null-terminated string will break the stringstream
    return str;
}

std::string ParallelUtils::GetDeviceInfo(cl_device_id device) {
    static const std::vector<std::tuple<cl_device_info, std::string>> DeviceInfoFields = {
        {CL_DEVICE_NAME, "Name"},
        {CL_DEVICE_VERSION, "Version"},
        {CL_DEVICE_PROFILE, "Profile"},
        {CL_DEVICE_VENDOR, "Vendor"},
        {CL_DRIVER_VERSION, "Driver Version"},
    };

    std::stringstream s;
    for (const auto& info : DeviceInfoFields) {
        cl_device_info infoId;
        std::string paramName;
        std::tie(infoId, paramName) = info;

        s << "  " << paramName <<
            ": " << GetDeviceInfoString(device, infoId).c_str() << std::endl;
    }

    cl_bool imageSupport;
    /*cl_int status = */clGetDeviceInfo(device, CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool), &imageSupport, nullptr);
    s << "  " << "Image Support: " << (imageSupport == CL_TRUE ? "Yes" : "No");

    return s.str();
}

std::string ParallelUtils::GetPlatformInfo(cl_platform_id platform) {
    static const std::vector<std::tuple<cl_platform_info, std::string>> PlatformInfoFields = {
        {CL_PLATFORM_NAME, "Name"},
        {CL_PLATFORM_VERSION, "Version"},
        {CL_PLATFORM_PROFILE, "Profile"},
        {CL_PLATFORM_VENDOR, "Vendor"},
    };

    std::stringstream s;
    for (const auto& info : PlatformInfoFields) {
        cl_platform_info infoId;
        std::string paramName;
        std::tie(infoId, paramName) = info;

        s << "  " << paramName <<
            ": " << GetPlatformInfoString(platform, infoId).c_str() << std::endl;
    }
    return s.str();
}

std::string ParallelUtils::GetOpenCLError(cl_int error) {
    switch (error) {
    // run-time and JIT compiler errors
    case CL_SUCCESS: return "CL_SUCCESS";
    case CL_DEVICE_NOT_FOUND: return "CL_DEVICE_NOT_FOUND";
    case CL_DEVICE_NOT_AVAILABLE: return "CL_DEVICE_NOT_AVAILABLE";
    case CL_COMPILER_NOT_AVAILABLE: return "CL_COMPILER_NOT_AVAILABLE";
    case CL_MEM_OBJECT_ALLOCATION_FAILURE: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case CL_OUT_OF_RESOURCES: return "CL_OUT_OF_RESOURCES";
    case CL_OUT_OF_HOST_MEMORY: return "CL_OUT_OF_HOST_MEMORY";
    case CL_PROFILING_INFO_NOT_AVAILABLE: return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case CL_MEM_COPY_OVERLAP: return "CL_MEM_COPY_OVERLAP";
    case CL_IMAGE_FORMAT_MISMATCH: return "CL_IMAGE_FORMAT_MISMATCH";
    case CL_IMAGE_FORMAT_NOT_SUPPORTED: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case CL_BUILD_PROGRAM_FAILURE: return "CL_BUILD_PROGRAM_FAILURE";
    case CL_MAP_FAILURE: return "CL_MAP_FAILURE";
    case CL_MISALIGNED_SUB_BUFFER_OFFSET: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
    case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
    case CL_COMPILE_PROGRAM_FAILURE: return "CL_COMPILE_PROGRAM_FAILURE";
    case CL_LINKER_NOT_AVAILABLE: return "CL_LINKER_NOT_AVAILABLE";
    case CL_LINK_PROGRAM_FAILURE: return "CL_LINK_PROGRAM_FAILURE";
    case CL_DEVICE_PARTITION_FAILED: return "CL_DEVICE_PARTITION_FAILED";
    case CL_KERNEL_ARG_INFO_NOT_AVAILABLE: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

    // compile-time errors
    case -30: return "CL_INVALID_VALUE";
    case -31: return "CL_INVALID_DEVICE_TYPE";
    case -32: return "CL_INVALID_PLATFORM";
    case -33: return "CL_INVALID_DEVICE";
    case -34: return "CL_INVALID_CONTEXT";
    case -35: return "CL_INVALID_QUEUE_PROPERTIES";
    case -36: return "CL_INVALID_COMMAND_QUEUE";
    case -37: return "CL_INVALID_HOST_PTR";
    case -38: return "CL_INVALID_MEM_OBJECT";
    case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case -40: return "CL_INVALID_IMAGE_SIZE";
    case -41: return "CL_INVALID_SAMPLER";
    case -42: return "CL_INVALID_BINARY";
    case -43: return "CL_INVALID_BUILD_OPTIONS";
    case -44: return "CL_INVALID_PROGRAM";
    case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
    case -46: return "CL_INVALID_KERNEL_NAME";
    case -47: return "CL_INVALID_KERNEL_DEFINITION";
    case -48: return "CL_INVALID_KERNEL";
    case -49: return "CL_INVALID_ARG_INDEX";
    case -50: return "CL_INVALID_ARG_VALUE";
    case -51: return "CL_INVALID_ARG_SIZE";
    case -52: return "CL_INVALID_KERNEL_ARGS";
    case -53: return "CL_INVALID_WORK_DIMENSION";
    case -54: return "CL_INVALID_WORK_GROUP_SIZE";
    case -55: return "CL_INVALID_WORK_ITEM_SIZE";
    case -56: return "CL_INVALID_GLOBAL_OFFSET";
    case -57: return "CL_INVALID_EVENT_WAIT_LIST";
    case -58: return "CL_INVALID_EVENT";
    case -59: return "CL_INVALID_OPERATION";
    case -60: return "CL_INVALID_GL_OBJECT";
    case -61: return "CL_INVALID_BUFFER_SIZE";
    case -62: return "CL_INVALID_MIP_LEVEL";
    case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
    case -64: return "CL_INVALID_PROPERTY";
    case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
    case -66: return "CL_INVALID_COMPILER_OPTIONS";
    case -67: return "CL_INVALID_LINKER_OPTIONS";
    case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

    // extension errors
    case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
    case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
    case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
    case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
    case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
    case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
    default: return "Unknown OpenCL error";
    }
}

static std::string GetOpenCLProgramBuildLog(cl_program program, cl_device_id device) {
    cl_int status;
    size_t buildLogLen;
    status = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &buildLogLen);
    if (status != CL_SUCCESS) {
        return "Failed to get program build log";
    }

    std::vector<char> buffer(buildLogLen);
    status = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, buildLogLen, buffer.data(), NULL);
    if (status != CL_SUCCESS) {
        return "Failed to get program build log";
    }

    std::string buildLog(buffer.begin(), buffer.end());
    return buildLog;
}

bool ParallelUtils::CreateProgram(cl_context context, cl_device_id device,
    const std::string& kernelName,
    const std::string& kernelSource,
    cl_program* program, cl_kernel* kernel) {

    cl_int status;

    const char* sourcePtr = kernelSource.c_str();
    cl_program newProgram = clCreateProgramWithSource(context, 1, &sourcePtr, NULL, &status);
    if (status != CL_SUCCESS) {
        LOGE << "Failed to create OpenCL program : " << GetOpenCLError(status);
        return false;
    }

    // Build the program
    status = clBuildProgram(newProgram, 0, NULL, NULL, NULL, NULL);
    if (status != CL_SUCCESS) {
        LOGE << "Failed to build OpenCL program : " << GetOpenCLError(status);
        LOGE << "OpenCL Build Log : " << std::endl << GetOpenCLProgramBuildLog(newProgram, device);
        return false;
    }

    // Create the kernel
    cl_kernel newKernel = clCreateKernel(newProgram, kernelName.c_str(), &status);
    if (status != CL_SUCCESS) {
        LOGE << "Failed to create OpenCL kernel : " << GetOpenCLError(status);
        return false;
    }

    *program = newProgram;
    *kernel = newKernel;

    return true;
}
