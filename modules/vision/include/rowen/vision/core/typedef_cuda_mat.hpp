#pragma once

#include <rowen/vision/core/typedef_matrix.hpp>

#if defined(RSDK_WITH_JETSON_UTILS)
  #include <cuda_runtime.h>
  #include <rowen_jetson/cudaRGB.h>  // from Jetson-utils

  #include <rowen/vision/core/typedef_size.hpp>
  #include <stdexcept>

  #define CUDA_DEBUG_LOG     false
  #define GPU_MAT_CUDASYNC() CUDA(cudaDeviceSynchronize())
#endif

namespace rs {
namespace vision {

#if defined(RSDK_WITH_JETSON_UTILS)
class GpuMat
{
  template <typename T = uchar3>
  void create(int _rows, int _cols, T* _data = nullptr)
  {
    // Release previous data
    release();

  #if !(CUDA_DEBUG_LOG)
    Log::SetLevel(Log::Level::SILENT);
  #endif
    cudaError_t res = cudaSuccess;

    // -- Generate new GpuMat ----------------------------------------------------
    const auto size = _cols * _rows * sizeof(uchar3);

    bool allocated = false;

    // allocation method
    const auto set_memory = [&](int kind = -1) {
      // Alloc memory
      if (data == nullptr)
      {
        res = cudaMalloc(&data, size);
        // res = cudaMallocManaged(&data, size);
        if (res != cudaSuccess)
        {
          throw std::runtime_error("GpuMat : create() : memory allocation failed");
        }
      }
      else
      {
        if (Size(cols, rows) != Size(_cols, _rows))
        {
          throw std::runtime_error("GpuMat : create() : size is not matched");
        }
      }
      allocated = true;

      // Copy memory
      if (kind >= 0)
      {
        res = cudaMemcpy(data, _data, size, (cudaMemcpyKind)kind);
      }
      else
      {
        res = cudaMemset(data, 0, size);
      }
      if (res != cudaSuccess)
      {
        throw std::runtime_error("GpuMat : create() : memory copy failed");
      }
    };

    // Allocate memory
    if (_data && size)
    {
      // If source is cv::Mat
      if constexpr (std::is_same<T, unsigned char>::value)
      {
        set_memory(cudaMemcpyHostToDevice);
      }
      else if constexpr (std::is_same<T, uchar3>::value)
      {
        set_memory(cudaMemcpyDeviceToDevice);
      }
      else
      {
        throw std::runtime_error("GpuMat : create() : Not supported type");
      }
    }
    else if (size)
    {
      set_memory();
    }

    if (allocated)
    {
      GPU_MAT_CUDASYNC();
    }

    cols = _cols;
    rows = _rows;
    if (ref == nullptr)
      ref = new int(1);
    else
      (*ref)++;
  }

 public:
  // --  GPU Matrix --------------------------------------------------------------
  GpuMat(int _rows = 0, int _cols = 0, uchar3* _data = nullptr)
  {
    create(_rows, _cols, _data);
  }

  GpuMat(const Size& _size, uchar3* _data = nullptr) : GpuMat(_size.height, _size.width, _data) {}

  // Shallower copy
  GpuMat(const GpuMat& other)
      : rows(other.rows), cols(other.cols), data(other.data), ref(other.ref)
  {
    if (ref)
    {
      (*ref)++;
    }
  }

  GpuMat(GpuMat&& other) noexcept
      : rows(other.rows), cols(other.cols), data(other.data), ref(other.ref)
  {
    if (ref)
    {
      (*ref)++;
    }
    other.release();
  }

  GpuMat& operator=(const GpuMat& other)
  {
    if (this != &other)
    {
      release();

      this->rows = other.rows;
      this->cols = other.cols;
      this->data = other.data;
      this->ref  = other.ref;

      if (ref)
      {
        (*ref)++;
      }
    }
    return *this;
  }

  GpuMat& operator=(GpuMat&& other) noexcept
  {
    if (this != &other)
    {
      release();
      rows = other.rows;
      cols = other.cols;
      data = other.data;
      ref  = other.ref;

      if (ref)
      {
        (*ref)++;
      }

      other.release();
    }
    return *this;
  }

  // Deep copy
  void copyTo(GpuMat& dest) const
  {
    if (this != &dest)
    {
      dest.release();
      dest.create(this->rows, this->cols, this->data);
    }
  }

  GpuMat clone() const
  {
    GpuMat res;
    this->copyTo(res);
    return res;
  }

  // -- GPU Matrix (from. cv::Mat) -----------------------------------------------
  GpuMat(unsigned char* cv_data, int _width, int _height)
  {
    create(_height, _width, cv_data);
  }

  GpuMat(const Size& _size, unsigned char* cv_data) : GpuMat(cv_data, _size.width, _size.height) {}

  GpuMat(const Mat& cv_mat) : GpuMat(cv_mat.data, cv_mat.cols, cv_mat.rows) {}

  GpuMat& operator=(const Mat& cv_mat)
  {
    release();
    create(cv_mat.rows, cv_mat.cols, cv_mat.data);
    return *this;
  }

  // GpuMat& operator=(Mat&& cv_mat);  // automatic defined by compiler

  // operator Mat() const // dangerous (when target Mat's data is exist)

  void copyTo(Mat& cv_mat) const
  {
    auto size = cols * rows * sizeof(uchar3);

    // if target Mat's data is not exist, then create new Mat
    if (cv_mat.data == nullptr)
    {
      cv_mat = Mat(rows, cols, CV_8UC3);
    }

    // copy data
    if (cv_mat.cols == cols && cv_mat.rows == rows)
    {
      auto res = cudaMemcpy(cv_mat.data, data, size, cudaMemcpyDeviceToHost);
      GPU_MAT_CUDASYNC();

      if (res != cudaSuccess)
      {
        throw std::runtime_error("GpuMat : cvMat() : CUDA memory copy failed.");
      }
    }
    else
    {
      throw std::runtime_error("GpuMat : cvMat() : size is not matched.");
    }
  }

  // -- Release ------------------------------------------------------------------
 public:
  ~GpuMat()
  {
    release();
  }

  void release()
  {
    if (ref)
    {
      (*ref)--;
      if (*ref == 0)
      {
        cudaFree(data);
        data = nullptr;

        delete ref;
        ref = nullptr;
      }
    }
    cols = rows = 0;
  }

  // -- Methods ------------------------------------------------------------------
  Size size() const { return Size(this->cols, this->rows); }

  bool empty() const { return data == nullptr; }

  // change self data
  void toRGB()
  {
    if (data == nullptr)
      return;
    if (cudaRGB8ToBGR8(data, data, cols, rows) == cudaSuccess)
    {
      GPU_MAT_CUDASYNC();
    }
  }
  void toBGR()
  {
    toRGB();
  }

  // set "res" data from self data
  void toRGB(GpuMat& res) const  // or to BGR
  {
    res.release();
    res.create(this->rows, this->cols);

    if (cudaRGB8ToBGR8(data, res.data, cols, rows) == cudaSuccess)
    {
      GPU_MAT_CUDASYNC();
    }
  }
  void toBGR(GpuMat& res) const
  {
    toRGB(res);
  }

 public:
  uchar3* data = nullptr;
  int     cols = 0;
  int     rows = 0;

  int* ref = nullptr;
};
#else
using GpuMat = Mat;
#endif

};  // namespace vision
};  // namespace rs
