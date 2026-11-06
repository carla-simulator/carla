// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "RawDataBuffer.h"
#include <gtest/gtest.h>

#include <memory>
#include <vector>

namespace {

  struct PythonDeleter {
    void operator()(PyObject *object) const { Py_XDECREF(object); }
  };
  using PythonObject = std::unique_ptr<PyObject, PythonDeleter>;
  using Payload = std::vector<unsigned char>;

  PythonObject NativeView(const std::shared_ptr<Payload> &payload, size_t offset = 0u) {
    PythonObject type(carla::python::detail::MakeRawDataBufferType());
    if (type == nullptr) {
      return nullptr;
    }
    auto *data = payload->data();
    if (offset != 0u) {
      data += offset;
    }
    return PythonObject(carla::python::detail::MakeRawDataMemoryView(
        type.get(), reinterpret_cast<char *>(data),
        static_cast<Py_ssize_t>(payload->size() - offset), payload));
  }

  class RawDataBuffer : public testing::Test {
  protected:
    static void SetUpTestSuite() { Py_Initialize(); }
    static void TearDownTestSuite() { Py_Finalize(); }
    void TearDown() override {
      EXPECT_FALSE(PyErr_Occurred());
      PyErr_Clear();
      PyGC_Collect();
    }
  };

  TEST_F(RawDataBuffer, private_exporter_rejects_direct_construction) {
    PythonObject type(carla::python::detail::MakeRawDataBufferType());
    ASSERT_NE(type, nullptr);
    PythonObject instance(PyObject_CallObject(type.get(), nullptr));
    EXPECT_EQ(instance, nullptr);
    EXPECT_TRUE(PyErr_ExceptionMatches(PyExc_TypeError));
    PyErr_Clear();
  }

  TEST_F(RawDataBuffer, view_retains_native_payload_and_releases_the_last_owner) {
    auto payload = std::make_shared<Payload>(Payload{1u, 2u, 3u});
    std::weak_ptr<Payload> observer = payload;
    auto view = NativeView(payload);
    ASSERT_NE(view, nullptr);
    payload.reset();
    ASSERT_FALSE(observer.expired());
    EXPECT_EQ(PyMemoryView_GET_BUFFER(view.get())->len, 3);
    view.reset();
    EXPECT_TRUE(observer.expired());
  }

  TEST_F(RawDataBuffer, slice_and_cast_retain_ownership_after_the_original_view_is_released) {
    auto payload = std::make_shared<Payload>(Payload{1u, 2u, 3u, 4u});
    std::weak_ptr<Payload> observer = payload;
    auto view = NativeView(payload);
    ASSERT_NE(view, nullptr);
    PythonObject slice(PySequence_GetSlice(view.get(), 1, 3));
    ASSERT_NE(slice, nullptr);
    PythonObject cast(PyObject_CallMethod(slice.get(), "cast", "s", "B"));
    ASSERT_NE(cast, nullptr);
    payload.reset();
    view.reset();
    slice.reset();
    ASSERT_FALSE(observer.expired());
    EXPECT_EQ(PyMemoryView_GET_BUFFER(cast.get())->len, 2);
    cast.reset();
    EXPECT_TRUE(observer.expired());
  }

  TEST_F(RawDataBuffer, readonly_contiguous_byte_buffer_rejects_a_writable_export) {
    auto payload = std::make_shared<Payload>(Payload{1u, 2u, 3u});
    auto view = NativeView(payload);
    ASSERT_NE(view, nullptr);
    Py_buffer readable{};
    ASSERT_EQ(PyObject_GetBuffer(view.get(), &readable, PyBUF_FULL_RO), 0);
    EXPECT_EQ(readable.readonly, 1);
    EXPECT_EQ(readable.ndim, 1);
    EXPECT_EQ(readable.itemsize, 1);
    EXPECT_STREQ(readable.format, "B");
    EXPECT_TRUE(PyBuffer_IsContiguous(&readable, 'C'));
    PyBuffer_Release(&readable);
    Py_buffer writable{};
    EXPECT_EQ(PyObject_GetBuffer(view.get(), &writable, PyBUF_WRITABLE), -1);
    EXPECT_TRUE(PyErr_ExceptionMatches(PyExc_BufferError));
    PyErr_Clear();
  }

  TEST_F(RawDataBuffer, payload_mutations_are_visible_without_copying_or_exposing_the_header) {
    auto payload = std::make_shared<Payload>(Payload{90u, 91u, 1u, 2u, 3u});
    auto view = NativeView(payload, 2u);
    ASSERT_NE(view, nullptr);
    auto *bytes = static_cast<unsigned char *>(PyMemoryView_GET_BUFFER(view.get())->buf);
    EXPECT_EQ(PyMemoryView_GET_BUFFER(view.get())->len, 3);
    EXPECT_EQ(bytes[0], 1u);
    (*payload)[2u] = 17u;
    EXPECT_EQ(bytes[0], 17u);
    EXPECT_EQ(bytes[2u], 3u);
  }

  TEST_F(RawDataBuffer, empty_payload_has_a_valid_readonly_empty_view) {
    auto payload = std::make_shared<Payload>();
    auto view = NativeView(payload);
    ASSERT_NE(view, nullptr);
    EXPECT_EQ(PyMemoryView_GET_BUFFER(view.get())->len, 0);
    EXPECT_EQ(PyMemoryView_GET_BUFFER(view.get())->readonly, 1);
  }

  TEST_F(RawDataBuffer, private_exporter_type_outlives_its_factory_reference) {
    auto payload = std::make_shared<Payload>(Payload{1u});
    // NativeView drops its local heap-type reference before returning.
    auto view = NativeView(payload);
    ASSERT_NE(view, nullptr);
    PyGC_Collect();
    PythonObject derived(PySequence_GetSlice(view.get(), 0, 1));
    ASSERT_NE(derived, nullptr);
    EXPECT_EQ(PyMemoryView_GET_BUFFER(derived.get())->len, 1);
  }

  TEST_F(RawDataBuffer, independent_exports_keep_storage_until_the_last_view_is_gone) {
    auto payload = std::make_shared<Payload>(Payload{1u, 2u});
    std::weak_ptr<Payload> observer = payload;
    auto first = NativeView(payload);
    auto second = NativeView(payload);
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    payload.reset();
    first.reset();
    EXPECT_FALSE(observer.expired());
    second.reset();
    EXPECT_TRUE(observer.expired());
  }

} // namespace
