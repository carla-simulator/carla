add_executable(test_async_server async_server.cpp)
target_link_libraries(test_async_server carla_server ${Boost_DATE_TIME_LIBRARY} ${Boost_REGEX_LIBRARY} ${Boost_SYSTEM_LIBRARY})

add_executable(test_sync_server sync_server.cpp)
target_link_libraries(test_sync_server carla_server ${Boost_SYSTEM_LIBRARY} ${Boost_DATE_TIME_LIBRARY} ${Boost_REGEX_LIBRARY})

add_executable(test_client client.cpp)
target_link_libraries(test_client carla_server libprotobuf libprotobuf-lite libprotoc ${Boost_SYSTEM_LIBRARY} ${Boost_DATE_TIME_LIBRARY} ${Boost_REGEX_LIBRARY})
