// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "carla/Exception.h"
#include "carla/BufferView.h"

namespace carla {
namespace sensor {
namespace data {

  /**
   * @brief Allocator to allow copyless conversion from an Image Serializer into a std::vector that uses the existing buffer data
   *        The prevents from calling memcpy() or similar.
  */
  template <typename T>
  class SerializerVectorAllocator: public std::allocator<T>
  {
  public:
    using std::allocator<T>::allocator;
    using pointer = typename std::allocator_traits<std::allocator<T>>::pointer;
    using size_type = typename std::allocator_traits<std::allocator<T>>::size_type;
    using const_pointer = typename std::allocator_traits<std::allocator<T>>::const_pointer;

    pointer allocate(size_type n, const void *hint=0)
    {
      (void)hint;
      if (_is_allocated) {
        carla::throw_exception(std::range_error("SerializerVectorAllocator:: memory already allocated"));
      }
      size_type const overall_size = sizeof(T)*n;
      if ( overall_size == _buffer->size() - _header_offset ) {
        return reinterpret_cast<pointer>(const_cast<carla::BufferView::value_type*>(_buffer->data() + _header_offset));
      }
      else {
        carla::throw_exception(std::range_error("SerializerVectorAllocator::allocate buffer size is " + std::to_string(_buffer->size()) 
          + " header offset is " + std::to_string(_header_offset) 
          + " but requested overall size is " + std::to_string(overall_size)));
      }

      return nullptr;
    }

    void deallocate(pointer p, size_type n)
    {
      (void)p;
      size_type const overall_size = sizeof(T)*n;
      if ( overall_size == _buffer->size() - _header_offset ) {
         _is_allocated = false;
      }
      else {
        carla::throw_exception(std::range_error("SerializerVectorAllocator::deallocate buffer size is " + std::to_string(_buffer->size()) 
          + " header offset is " + std::to_string(_header_offset) 
          + " but requested overall size is " + std::to_string(overall_size)));
      }
    }

    /**
     * nothing is initialized because it would overwrite the buffer data
    */
    template <class U, class... Args> void construct(U*, Args&&...) {
    }

    SerializerVectorAllocator(const carla::SharedBufferView buffer, size_type header_offset) : 
      std::allocator<T>(),
      _buffer(buffer),
      _header_offset(header_offset) { 
    }
    ~SerializerVectorAllocator() = default;

  private:
    carla::SharedBufferView _buffer;
    size_type _header_offset {0u};
    bool _is_allocated{false};
  };

  /**
   * @brief calculates the number of elements of the buffer view by reducing the buffer size by the provided header_offset and division by sizeof(T)
  */
  template <typename T>
  std::size_t number_of_elements(const carla::SharedBufferView buffer, size_t header_offset) {
      return (buffer->size() - header_offset) / sizeof(T);
  }

  /**
   * @brief create a vector with custom allocator providing the buffer memory
   * 
   * This provides std::vector access to the buffer data while leaving out the header_offset.
   * The vector size is deduced from number_of_elements().
   * This is a copyless operation, but the vector data cannot be assigned/moved to a std::vector with standard allocator without copy.
  */
  template <typename T>
  std::vector<T, carla::sensor::data::SerializerVectorAllocator<T>> buffer_data_accessed_by_vector(const carla::SharedBufferView buffer_view, size_t header_offset) {
      auto number_of_elements =  carla::sensor::data::number_of_elements<T>(buffer_view, header_offset);
      std::vector<T, carla::sensor::data::SerializerVectorAllocator<T>> vector_data(
        number_of_elements, 
        std::move(carla::sensor::data::SerializerVectorAllocator<T>(buffer_view, header_offset)));
      return vector_data;
  }

 
  /**
   * @brief create a vector with default allocator and copy the data from buffer memory
   * 
   * This provides std::vector copy of the buffer data while leaving out the carla::sensor::s11n::ImageSerializer::header_offset.
   * The vector size is deduced from number_of_elements().
   * This is a operation performing copy operation, but the vector data cannot be assigned/moved to a std::vector with standard allocator without copy.
  */
  template <typename T>
  std::vector<T> buffer_data_copy_to_std_vector(const carla::SharedBufferView buffer_view, size_t header_offset) {
      auto buffer_data = buffer_data_accessed_by_vector<T>(buffer_view, header_offset);
      std::vector<T> vector_data(buffer_data.begin(), buffer_data.end());
      return vector_data;
  }

} // namespace data
} // namespace sensor
} // namespace carla
