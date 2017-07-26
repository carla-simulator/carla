if (CMAKE_BUILD_TYPE MATCHES DebugPNG OR CMAKE_BUILD_TYPE MATCHES ReleasePNG)

  add_executable(test_async_server_png async_server.cpp)
  target_link_libraries(test_async_server_png ${CarlaServer_LIBRARIES})

  add_executable(test_sync_server_png sync_server.cpp)
  target_link_libraries(test_sync_server_png ${CarlaServer_LIBRARIES})

  add_executable(test_client_png client.cpp)
  target_link_libraries(test_client_png ${CarlaServer_LIBRARIES})

else (CMAKE_BUILD_TYPE MATCHES DebugPNG OR CMAKE_BUILD_TYPE MATCHES ReleasePNG)

  add_executable(test_async_server async_server.cpp)
  target_link_libraries(test_async_server ${CarlaServer_LIBRARIES})

  add_executable(test_sync_server sync_server.cpp)
  target_link_libraries(test_sync_server ${CarlaServer_LIBRARIES})

  add_executable(test_client client.cpp)
  target_link_libraries(test_client ${CarlaServer_LIBRARIES})

endif (CMAKE_BUILD_TYPE MATCHES DebugPNG OR CMAKE_BUILD_TYPE MATCHES ReleasePNG)
