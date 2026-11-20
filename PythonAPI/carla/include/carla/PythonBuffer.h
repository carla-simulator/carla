// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/PythonUtil.h"

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace carla {
namespace python_buffer {

  namespace bp = boost::python;

  /// Reads @a object, a C-contiguous buffer (numpy array, array.array,
  /// bytes...) of any integer or floating-point type or a sequence of
  /// numbers or of sequences of numbers, as @a rows rows of @a columns values
  /// of type T. @a rows of 0 takes the rows from the data.
  template <typename T>
  inline std::vector<T> ReadValues(const bp::object &object, size_t &rows, size_t columns, const char *name) {
    std::vector<T> values;
    PyObject *ptr = object.ptr();
    if (PyObject_CheckBuffer(ptr)) {
      Py_buffer view;
      if (PyObject_GetBuffer(ptr, &view, PyBUF_C_CONTIGUOUS | PyBUF_FORMAT) != 0) {
        bp::throw_error_already_set();
      }
      struct Release {
        Py_buffer *view;
        ~Release() { PyBuffer_Release(view); }
      } release{&view};
      const char *format = view.format != nullptr ? view.format : "B";
      if ((*format == '<') || (*format == '=') || (*format == '@')) {
        ++format;
      }
      const size_t count = static_cast<size_t>(view.len / view.itemsize);
      values.resize(count);
      auto copy = [&](auto tag) {
        using S = decltype(tag);
        if (static_cast<size_t>(view.itemsize) != sizeof(S)) {
          throw std::invalid_argument(std::string(name) + ": unexpected item size");
        }
        const auto *src = static_cast<const uint8_t *>(view.buf);
        for (size_t i = 0u; i < count; ++i) {
          S value;
          std::memcpy(&value, src + i * sizeof(S), sizeof(S));
          values[i] = static_cast<T>(value);
        }
      };
      switch (format[0]) {
        case 'd': copy(double{}); break;
        case 'f': copy(float{}); break;
        case 'b': copy(int8_t{}); break;
        case 'B': copy(uint8_t{}); break;
        case '?': copy(uint8_t{}); break;
        case 'h': copy(int16_t{}); break;
        case 'H': copy(uint16_t{}); break;
        case 'i': copy(int32_t{}); break;
        case 'I': copy(uint32_t{}); break;
        case 'l': view.itemsize == 8 ? copy(int64_t{}) : copy(int32_t{}); break;
        case 'L': view.itemsize == 8 ? copy(uint64_t{}) : copy(uint32_t{}); break;
        case 'q': copy(int64_t{}); break;
        case 'Q': copy(uint64_t{}); break;
        default:
          throw std::invalid_argument(
              std::string(name) + ": unsupported buffer format '" + format + "'");
      }
    } else {
      // A Python sequence of numbers, or of rows of numbers.
      const auto length = bp::len(object);
      for (decltype(bp::len(object)) i = 0; i < length; ++i) {
        bp::object item = object[i];
        if ((columns > 1u) || PySequence_Check(item.ptr())) {
          const auto width = bp::len(item);
          for (decltype(bp::len(item)) j = 0; j < width; ++j) {
            values.push_back(bp::extract<T>(item[j]));
          }
        } else {
          values.push_back(bp::extract<T>(item));
        }
      }
    }
    if (rows == 0u) {
      if (values.size() % columns != 0u) {
        throw std::invalid_argument(
            std::string(name) + ": expected " + std::to_string(columns) + " values per actor");
      }
      rows = values.size() / columns;
    } else if (values.size() != rows * columns) {
      throw std::invalid_argument(
          std::string(name) + ": expected " + std::to_string(rows) + " x " +
          std::to_string(columns) + " values, got " + std::to_string(values.size()));
    }
    return values;
  }

  /// A writable C-contiguous buffer of exactly @a count values of type T and
  /// buffer format @a Format (a numpy array of the matching dtype), held for
  /// writing.
  template <typename T, char Format>
  class WritableBuffer {
  public:

    WritableBuffer(const bp::object &object, size_t count, const char *name) {
      if (PyObject_GetBuffer(object.ptr(), &_view, PyBUF_C_CONTIGUOUS | PyBUF_FORMAT | PyBUF_WRITABLE) != 0) {
        bp::throw_error_already_set();
      }
      const char *format = _view.format != nullptr ? _view.format : "B";
      if ((*format == '<') || (*format == '=') || (*format == '@')) {
        ++format;
      }
      if ((format[0] != Format) || (format[1] != '\0') ||
          (static_cast<size_t>(_view.itemsize) != sizeof(T)) ||
          (static_cast<size_t>(_view.len) != count * sizeof(T))) {
        PyBuffer_Release(&_view);
        throw std::invalid_argument(
            std::string(name) + ": expected a C-contiguous writable array of " +
            std::to_string(count) + " values of format '" + Format + "'");
      }
    }

    ~WritableBuffer() {
      PyBuffer_Release(&_view);
    }

    WritableBuffer(const WritableBuffer &) = delete;
    WritableBuffer &operator=(const WritableBuffer &) = delete;

    T *data() const {
      return static_cast<T *>(_view.buf);
    }

  private:

    Py_buffer _view;
  };

  /// numpy float64.
  using WritableDoubles = WritableBuffer<double, 'd'>;

  /// numpy bool.
  using WritableBools = WritableBuffer<uint8_t, '?'>;

  /// numpy uint32.
  using WritableUInt32 = WritableBuffer<uint32_t, 'I'>;

} // namespace python_buffer
} // namespace carla
