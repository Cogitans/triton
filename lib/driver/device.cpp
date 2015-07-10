#include "isaac/driver/device.h"
#include <algorithm>

namespace isaac
{

namespace driver
{


#ifdef ISAAC_WITH_CUDA
template<CUdevice_attribute attr>
int Device::cuGetInfo() const
{
  int res;
  cuda::check(cuDeviceGetAttribute(&res, attr, *h_.cu));
  return res;
}

Device::Device(int ordinal): backend_(CUDA), h_(backend_)
{ cuda::check(cuDeviceGet(h_.cu.get(), ordinal)); }

#endif


Device::Device(cl::Device const & device) : backend_(OPENCL), h_(backend_)
{ *h_.cl = device; }

backend_type Device::backend() const
{ return backend_; }

unsigned int Device::address_bits() const
{
  switch(backend_)
  {
#ifdef ISAAC_WITH_CUDA
    case CUDA: return sizeof(long long)*8;
#endif
    case OPENCL: return h_.cl->getInfo<CL_DEVICE_ADDRESS_BITS>();
    default: throw;
  }

  return backend_;
}

driver::Platform Device::platform() const
{
  switch(backend_)
  {
#ifdef ISAAC_WITH_CUDA
    case CUDA: return Platform(CUDA);
#endif
    case OPENCL: return Platform(h_.cl->getInfo<CL_DEVICE_PLATFORM>());
    default: throw;
  }
}

std::string Device::name() const
{
  switch(backend_)
  {
#ifdef ISAAC_WITH_CUDA
    case CUDA:
      char tmp[128];
      cuda::check(cuDeviceGetName(tmp, 128, *h_.cu));
      return std::string(tmp);
#endif
    case OPENCL: return h_.cl->getInfo<CL_DEVICE_NAME>();
    default: throw;
  }
}

std::string Device::vendor_str() const
{
  switch(backend_)
  {
#ifdef ISAAC_WITH_CUDA
    case CUDA: return "NVidia";
#endif
    case OPENCL: return h_.cl->getInfo<CL_DEVICE_VENDOR>();
    default: throw;
  }
}

Device::VENDOR Device::vendor() const
{
    std::string vname = vendor_str();
    std::transform(vname.begin(), vname.end(), vname.begin(), ::tolower);
    if(vname.find("nvidia")!=std::string::npos)
        return NVIDIA;
    else if(vname.find("intel")!=std::string::npos)
        return INTEL;
    else if(vname.find("amd")!=std::string::npos || vname.find("advanced micro devices")!=std::string::npos)
        return AMD;
    else
        return UNKNOWN;
}

std::vector<size_t> Device::max_work_item_sizes() const
{
  switch(backend_)
  {
#ifdef ISAAC_WITH_CUDA
    case CUDA:
    {
      std::vector<size_t> result(3);
      result[0] = cuGetInfo<CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_X>();
      result[1] = cuGetInfo<CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Y>();
      result[2] = cuGetInfo<CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Z>();
      return result;
    }
#endif
    case OPENCL:
      return h_.cl->getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
    default:
      throw;
  }
}

device_type Device::type() const
{
  switch(backend_)
  {
#ifdef ISAAC_WITH_CUDA
    case CUDA: return DEVICE_TYPE_GPU;
#endif
    case OPENCL: return static_cast<device_type>(h_.cl->getInfo<CL_DEVICE_TYPE>());
    default: throw;
  }
}

std::string Device::extensions() const
{
  switch(backend_)
  {
#ifdef ISAAC_WITH_CUDA
    case CUDA:
      return "";
#endif
    case OPENCL:
      return h_.cl->getInfo<CL_DEVICE_EXTENSIONS>();
    default: throw;
  }
}

#ifdef ISAAC_WITH_CUDA
    #define CUDACASE(CUNAME) case CUDA: return cuGetInfo<CUNAME>();
#else
    #define CUDACASE(CUNAME)
#endif\

#define WRAP_ATTRIBUTE(ret, fname, CUNAME, CLNAME) \
  ret Device::fname() const\
  {\
    switch(backend_)\
    {\
      CUDACASE(CUNAME)\
      case OPENCL: return h_.cl->getInfo<CLNAME>();\
      default: throw;\
    }\
  }\


WRAP_ATTRIBUTE(size_t, max_work_group_size, CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK, CL_DEVICE_MAX_WORK_GROUP_SIZE)
WRAP_ATTRIBUTE(size_t, local_mem_size, CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK, CL_DEVICE_LOCAL_MEM_SIZE)
WRAP_ATTRIBUTE(size_t, warp_wavefront_size, CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK, CL_DEVICE_WAVEFRONT_WIDTH_AMD)
WRAP_ATTRIBUTE(size_t, clock_rate, CU_DEVICE_ATTRIBUTE_CLOCK_RATE, CL_DEVICE_MAX_CLOCK_FREQUENCY)


std::pair<unsigned int, unsigned int> Device::nv_compute_capability() const
{
  switch(backend_)
  {
      case OPENCL:
          return std::pair<unsigned int, unsigned int>( h_.cl->getInfo<CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV>(), h_.cl->getInfo<CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV> ());
#ifdef ISAAC_WITH_CUDA
      case CUDA:
          return std::pair<unsigned int, unsigned int>(cuGetInfo<CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR>(), cuGetInfo<CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR>());
#endif
      default:
          throw;
  }


}



}

}
