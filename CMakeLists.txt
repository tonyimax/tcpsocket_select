cmake_minimum_required(VERSION 3.17)
project(tcpsocket)

set(CMAKE_CXX_STANDARD 14)

#set(CXX_FLAGS -g -fpermissive)

#string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

#add_executable(server_concurrent  concurrent_server.cpp)

#add_executable(client  client.cpp)

#target_link_libraries(server_concurrent pthread)

#add_executable(select_server select_server.cpp)

add_executable(select_test select_test.cpp)