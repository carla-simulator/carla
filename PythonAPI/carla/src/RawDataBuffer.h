// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <Python.h>

#include <memory>
#include <utility>

namespace carla {
namespace python {
namespace detail {

  struct RawDataBuffer {
    PyObject_HEAD
    std::shared_ptr<void> *owner;
    char *data;
    Py_ssize_t size;
  };

  inline void DestroyRawDataBuffer(PyObject *object) {
    auto *self = reinterpret_cast<RawDataBuffer *>(object);
    PyTypeObject *type = Py_TYPE(object);
    delete self->owner;
    type->tp_free(object);
    Py_DECREF(type);
  }

  inline int ExportRawDataBuffer(PyObject *object, Py_buffer *view, int flags) {
    const auto *self = reinterpret_cast<RawDataBuffer *>(object);
    return PyBuffer_FillInfo(view, object, self->data, self->size, 1, flags);
  }

  inline PyObject *RejectRawDataBufferConstruction(PyTypeObject *, PyObject *, PyObject *) {
    PyErr_SetString(PyExc_TypeError, "Raw-data buffers are created by sensor data objects");
    return nullptr;
  }

  inline PyObject *MakeRawDataBufferType() {
    static PyType_Slot slots[] = {
        {Py_tp_new, reinterpret_cast<void *>(RejectRawDataBufferConstruction)},
        {Py_tp_dealloc, reinterpret_cast<void *>(DestroyRawDataBuffer)},
        {Py_bf_getbuffer, reinterpret_cast<void *>(ExportRawDataBuffer)},
        {0, nullptr}};
    PyType_Spec spec = {
        "carla._RawDataBuffer", static_cast<int>(sizeof(RawDataBuffer)), 0,
        Py_TPFLAGS_DEFAULT, slots};
    return PyType_FromSpec(&spec);
  }

  inline PyObject *MakeRawDataMemoryView(
      PyObject *buffer_type, char *data, Py_ssize_t size, std::shared_ptr<void> owner) {
    // Allocate C++ ownership before a Python object needs exception cleanup.
    auto holder = std::make_unique<std::shared_ptr<void>>(std::move(owner));
    auto *type = reinterpret_cast<PyTypeObject *>(buffer_type);
    auto *self = reinterpret_cast<RawDataBuffer *>(type->tp_alloc(type, 0));
    if (self == nullptr) {
      return nullptr;
    }
    self->owner = holder.release();
    self->data = data;
    self->size = size;
    auto *object = reinterpret_cast<PyObject *>(self);
    PyObject *view = PyMemoryView_FromObject(object);
    Py_DECREF(object);
    return view;
  }

} // namespace detail
} // namespace python
} // namespace carla
