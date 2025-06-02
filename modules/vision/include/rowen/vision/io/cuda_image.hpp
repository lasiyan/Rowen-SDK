#ifndef RS_VISION_IO_CUDA_IMAGE_HPP
#define RS_VISION_IO_CUDA_IMAGE_HPP

#include <rowen_jetson/imageIO.h>

#include <rowen/vision/type_traits_cuda.hpp>

namespace rs {
namespace vision {
namespace io {

class image;  // forward declaration

namespace image_impl {

class cuda
{
  friend class rs::vision::io::image;

  static rs::GpuMat read(const std::string& filename, imageFormat format)
  {
    GpuMat img;
    if (!loadImage(filename.c_str(), &img.data, &img.cols, &img.rows))
    {
      // printf("[debugging] failed to load image '%s'\n", filename.c_str());
    }
    return img;
  }

  static bool write(const std::string& filename, const GpuMat& img)
  {
    cudaPointerAttributes attributes;

    bool need_to_host_memory = false;

    if (cudaPointerGetAttributes(&attributes, (uchar3*)img.data) == cudaSuccess)
    {
      if (attributes.type == cudaMemoryTypeDevice)
      {
        need_to_host_memory = true;
      }
    }
    else
    {
      need_to_host_memory = true;
    }

    uchar3* host_data = nullptr;
    bool    result    = false;

    if (need_to_host_memory)
    {
      const auto size = img.cols * img.rows * sizeof(uchar3);
      try
      {
        if (cudaHostAlloc((void**)&host_data, size, cudaHostAllocMapped) != cudaSuccess)
          throw std::runtime_error("failed to allocate host memory");

        if (cudaMemcpy(host_data, img.data, size, cudaMemcpyDeviceToHost) != cudaSuccess)
          throw std::runtime_error("failed to copy device memory to host");
      }
      catch (...)
      {
        result = false;
      }
    }
    else
    {
      host_data = img.data;
    }

    if (host_data)
      result = saveImage(filename.c_str(), host_data, img.cols, img.rows);

    if (need_to_host_memory && host_data)
      cudaFreeHost(host_data);

    return result;
  }
};

};  // namespace image_impl
};  // namespace io
};  // namespace vision
};  // namespace rs

#endif  // RS_VISION_IO_CUDA_IMAGE_HPP
