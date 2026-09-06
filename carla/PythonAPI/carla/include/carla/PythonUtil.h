// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//
// Architectural correction: this header was previously LibCarla/source/carla/PythonUtil.h, gated behind LIBCARLA_WITH_PYTHON_SUPPORT and pulling Boost::python into the otherwise-Python-free C++ client (carla-client). It only ever served PythonAPI bindings, so it was relocated here. LibCarla now has zero Python touchpoints; carla-client builds without any Python dependency. PythonAPI continues to use these GIL helpers as before.

#pragma once

#include "carla/NonCopyable.h"

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wdeprecated-register"
#endif
#include <boost/python.hpp>
#if defined(__clang__)
#  pragma clang diagnostic pop
#endif

namespace carla {

  class PythonUtil {
  public:

    static bool ThisThreadHasTheGIL() {
#if PY_MAJOR_VERSION >= 3
      return PyGILState_Check();
#else
      PyThreadState *tstate = _PyThreadState_Current;
      return (tstate != nullptr) && (tstate == PyGILState_GetThisThreadState());
#endif
    }

    class AcquireGIL : private NonCopyable {
    public:

      AcquireGIL() : _state(PyGILState_Ensure()) {}

      ~AcquireGIL() {
        PyGILState_Release(_state);
      }

    private:

      PyGILState_STATE _state;
    };

    class ReleaseGIL : private NonCopyable {
    public:

      ReleaseGIL() : _state(PyEval_SaveThread()) {}

      ~ReleaseGIL() {
        PyEval_RestoreThread(_state);
      }

    private:

      PyThreadState *_state;
    };

    class AcquireGILDeleter {
    public:

      template <typename T>
      void operator()(T *ptr) const {
        if (ptr != nullptr && !PythonUtil::ThisThreadHasTheGIL()) {
          AcquireGIL lock;
          delete ptr;
        } else {
          delete ptr;
        }
      }
    };

    class ReleaseGILDeleter {
    public:

      template <typename T>
      void operator()(T *ptr) const {
        if (ptr != nullptr && PythonUtil::ThisThreadHasTheGIL()) {
          ReleaseGIL lock;
          delete ptr;
        } else {
          delete ptr;
        }
      }
    };
  };

} // namespace carla
