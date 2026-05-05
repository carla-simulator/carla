carla_message ("CARLA Building application layer")

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/CarlaStudio")
  add_subdirectory (CarlaStudio)
endif ()
