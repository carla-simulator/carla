add_executable(test_async_server async_server.cpp)
target_link_libraries(test_async_server ${CarlaServer_LIBRARIES})

add_executable(test_sync_server sync_server.cpp)
target_link_libraries(test_sync_server ${CarlaServer_LIBRARIES})

add_executable(test_client client.cpp)
target_link_libraries(test_client ${CarlaServer_LIBRARIES})
