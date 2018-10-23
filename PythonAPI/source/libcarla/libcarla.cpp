// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/PythonUtil.h>

#include <type_traits>

// Convenient for requests without arguments.
#define CALL_WITHOUT_GIL(cls, fn) +[](cls &self) { \
      carla::PythonUtil::ReleaseGIL unlock; \
      return self.fn(); \
    }

// Convenient for const requests without arguments.
#define CONST_CALL_WITHOUT_GIL(cls, fn) CALL_WITHOUT_GIL(const cls, fn)

// Convenient for const requests that need to make a copy of the returned value.
#define CALL_RETURNING_COPY(cls, fn) +[](const cls &self) \
        -> std::decay_t<std::result_of_t<decltype(&cls::fn)(cls*)>> { \
      return self.fn(); \
    }

// Convenient for const requests that need to make a copy of the returned value.
#define CALL_RETURNING_COPY_1(cls, fn, _T1) +[](const cls &self, _T1 t1) \
        -> std::decay_t<std::result_of_t<decltype(&cls::fn)(cls*, _T1)>> { \
      return self.fn(std::forward<_T1>(t1)); \
    }

#include "Actor.cpp"
#include "Blueprint.cpp"
#include "Client.cpp"
#include "Control.cpp"
#include "Exception.cpp"
#include "Sensor.cpp"
#include "SensorData.cpp"
#include "Transform.cpp"
#include "World.cpp"

BOOST_PYTHON_MODULE(libcarla) {
  using namespace boost::python;
  PyEval_InitThreads();
  scope().attr("__path__") = "libcarla";
  export_transform();
  export_control();
  export_blueprint();
  export_actor();
  export_sensor();
  export_sensor_data();
  export_world();
  export_client();
  export_exception();
}
