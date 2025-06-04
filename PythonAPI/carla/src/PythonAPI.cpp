// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <PythonAPI.h>

extern void export_geom();
extern void export_control();
extern void export_blueprint();
extern void export_actor();
extern void export_sensor();
extern void export_sensor_data();
extern void export_snapshot();
extern void export_weather();
extern void export_world();
extern void export_map();
extern void export_client();
extern void export_exception();
extern void export_commands();
extern void export_trafficmanager();
extern void export_lightmanager();
#ifdef LIBCARLA_RSS_ENABLED
extern void export_ad_rss();
#endif
#ifdef CARLA_PYTHON_API_HAS_OSM2ODR
extern void export_osm2odr();
#endif

BOOST_PYTHON_MODULE(carla) {
  using namespace boost::python;
#if PY_MAJOR_VERSION < 3 || PY_MINOR_VERSION < 7
  PyEval_InitThreads();
#endif
  export_geom();
  export_control();
  export_blueprint();
  export_actor();
  export_sensor();
  export_sensor_data();
  export_snapshot();
  export_weather();
  export_world();
  export_map();
  export_client();
  export_exception();
  export_commands();
  export_trafficmanager();
  export_lightmanager();
#ifdef LIBCARLA_RSS_ENABLED
  export_ad_rss();
#endif
#ifdef CARLA_PYTHON_API_HAS_OSM2ODR
  export_osm2odr();
#endif
}
