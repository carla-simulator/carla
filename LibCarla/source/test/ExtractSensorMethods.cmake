# Execute the real sensor publishing methods in the host-side fixture without
# requiring an Unreal build. Only their surrounding engine/DDS interfaces are inert.
function(carla_extract_sensor_method file class method output)
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${file}")
  file(READ "${file}" source)
  string(FIND "${source}" "void ${class}::${method}(" start)
  if(start LESS 0)
    message(FATAL_ERROR "Cannot find ${class}::${method} in ${file}")
  endif()
  string(SUBSTRING "${source}" ${start} -1 tail)
  string(FIND "${tail}" "{" cursor)
  if(cursor LESS 0)
    message(FATAL_ERROR "Missing body for ${class}::${method}")
  endif()
  math(EXPR cursor "${cursor} + 1")
  set(depth 1)
  string(LENGTH "${tail}" length)
  while(depth GREATER 0 AND cursor LESS length)
    string(SUBSTRING "${tail}" ${cursor} 1 character)
    if(character STREQUAL "{")
      math(EXPR depth "${depth} + 1")
    elseif(character STREQUAL "}")
      math(EXPR depth "${depth} - 1")
    endif()
    math(EXPR cursor "${cursor} + 1")
  endwhile()
  if(NOT depth EQUAL 0)
    message(FATAL_ERROR "Unbalanced body for ${class}::${method}")
  endif()
  string(SUBSTRING "${tail}" 0 ${cursor} body)
  set(${output} "${body}" PARENT_SCOPE)
endfunction()

set(sensor_methods "")
foreach(record
    "RayCastLidar|ARayCastLidar|SendData"
    "RayCastSemanticLidar|ARayCastSemanticLidar|SendData"
    "HSSLidar|AHSSLidar|SendData"
    "Radar|ARadar|PostPhysTick")
  string(REPLACE "|" ";" fields "${record}")
  list(GET fields 0 file)
  list(GET fields 1 class)
  list(GET fields 2 method)
  carla_extract_sensor_method(
    "${CARLA_WORKSPACE_PATH}/Unreal/CarlaUnreal/Plugins/Carla/Source/Carla/Sensor/${file}.cpp"
    "${class}" "${method}" body)
  string(APPEND sensor_methods "${body}\n")
endforeach()
file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/sensor_ros_methods.inc"
  CONTENT "${sensor_methods}" @ONLY)
set_property(SOURCE "${LIBCARLA_SOURCE_PATH}/test/common/test_sensor_ros_publishing.cpp"
  APPEND PROPERTY INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}")
