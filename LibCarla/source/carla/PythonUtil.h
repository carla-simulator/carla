// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"

#ifdef LIBCARLA_WITH_PYTHON_SUPPORT
#  if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wdeprecated-register"
#  endif
#    include <boost/python.hpp>
#  if defined(__clang__)
#    pragma clang diagnostic pop
#  endif
#endif // LIBCARLA_WITH_PYTHON_SUPPORT

namespace carla {

  class PythonUtil {
  public:

    static bool ThisThreadHasTheGIL() {
#ifdef LIBCARLA_WITH_PYTHON_SUPPORT
#  if PY_MAJOR_VERSION >= 3
      return PyGILState_Check();
#  else
      PyThreadState *tstate = _PyThreadState_Current;
      return (tstate != nullptr) && (tstate == PyGILState_GetThisThreadState());
#  endif // PYTHON3
#else
      return false;
#endif // LIBCARLA_WITH_PYTHON_SUPPORT
    }

#ifdef LIBCARLA_WITH_PYTHON_SUPPORT

    /// Acquires a lock on the Python's Global Interpreter Lock, necessary for
    /// calling Python code from a different thread.
    class AcquireGIL : private NonCopyable {
    public:

      AcquireGIL() : _state(PyGILState_Ensure()) {}

      ~AcquireGIL() {
        PyGILState_Release(_state);
      }

    private:

      PyGILState_STATE _state;
    };

    /// Releases the lock on the Python's Global Interpreter Lock, use it when doing
    /// blocking I/O operations.
    class ReleaseGIL : private NonCopyable {
    public:

      ReleaseGIL() : _state(PyEval_SaveThread()) {}

      ~ReleaseGIL() {
        PyEval_RestoreThread(_state);
      }

    private:

      PyThreadState *_state;
    };

#else // LIBCARLA_WITH_PYTHON_SUPPORT

    class AcquireGIL : private NonCopyable {};
    class ReleaseGIL : private NonCopyable {};

#endif // LIBCARLA_WITH_PYTHON_SUPPORT

    /// A deleter that can be passed to a smart pointer to acquire the GIL
    /// before destroying the object.
    class AcquireGILDeleter {
    public:

      template <typename T>
      void operator()(T *ptr) const {
#ifdef LIBCARLA_WITH_PYTHON_SUPPORT
        if (ptr != nullptr && !PythonUtil::ThisThreadHasTheGIL()) {
          AcquireGIL lock;
          delete ptr;
        } else
#endif // LIBCARLA_WITH_PYTHON_SUPPORT
        delete ptr;
      }
    };

    /// A deleter that can be passed to a smart pointer to release the GIL
    /// before destroying the object.
    class ReleaseGILDeleter {
    public:

      template <typename T>
      void operator()(T *ptr) const {
#ifdef LIBCARLA_WITH_PYTHON_SUPPORT
        if (ptr != nullptr && PythonUtil::ThisThreadHasTheGIL()) {
          ReleaseGIL lock;
          delete ptr;
        } else
#endif // LIBCARLA_WITH_PYTHON_SUPPORT
        delete ptr;
      }
    };
  };

} // namespace carla
