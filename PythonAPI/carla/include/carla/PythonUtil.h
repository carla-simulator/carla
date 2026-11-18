// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
//
// Architectural correction: this header was previously LibCarla/source/carla/PythonUtil.h, gated behind LIBCARLA_WITH_PYTHON_SUPPORT and pulling Boost::python into the otherwise-Python-free C++ client (carla-client). It only ever served PythonAPI bindings, so it was relocated here. LibCarla now has zero Python touchpoints; carla-client builds without any Python dependency. PythonAPI continues to use these GIL helpers as before.

#pragma once

#include "carla/NonCopyable.h"

#include <atomic>
#include <chrono>
#include <thread>

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

    /// CARLA's client threads keep delivering world ticks and sensor data
    /// while Python shuts down, because nothing guarantees that the Client
    /// is destroyed before the interpreter is. Entering a finalized
    /// interpreter (PyGILState_Ensure) crashes the process, e.g. with
    /// "Fatal Python error: gilstate_tss_set" or a segmentation fault.
    ///
    /// Code that enters Python from a CARLA thread first opens a
    /// PythonCallScope and gives up if it is refused. The carla module
    /// registers OnInterpreterExit() with atexit, which runs at the start of
    /// Py_FinalizeEx while the interpreter is still alive: from then on every
    /// new scope is refused, and the scopes already open are waited for.
    class PythonCallScope : private NonCopyable {
    public:

      // Sequentially consistent on both sides: either OnInterpreterExit()
      // sees this scope in InFlight(), or this scope sees Exiting().
      PythonCallScope() {
        InFlight().fetch_add(1);
        _allowed = !Exiting().load();
      }

      ~PythonCallScope() {
        InFlight().fetch_sub(1);
      }

      explicit operator bool() const {
        return _allowed;
      }

    private:

      bool _allowed;
    };

    /// Registered with atexit by the carla module; called with the GIL held.
    static void OnInterpreterExit() {
      Exiting().store(true);
      // Let the callbacks already inside finish; they need the GIL.
      ReleaseGIL unlock;
      const auto deadline =
          std::chrono::steady_clock::now() + std::chrono::seconds(5);
      while (InFlight().load() > 0 &&
             std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    }

    class AcquireGILDeleter {
    public:

      template <typename T>
      void operator()(T *ptr) const {
        if (ptr == nullptr) {
          return;
        }
        PythonCallScope scope;
        if (!scope) {
          // The interpreter is exiting: leave the object to the operating
          // system instead of touching Python.
          return;
        }
        if (!PythonUtil::ThisThreadHasTheGIL()) {
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

  private:

    static std::atomic_bool &Exiting() {
      static std::atomic_bool exiting{false};
      return exiting;
    }

    static std::atomic_int &InFlight() {
      static std::atomic_int in_flight{0};
      return in_flight;
    }
  };

} // namespace carla
