cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

project(example
        LANGUAGES C CXX
        DESCRIPTION "test"
        HOMEPAGE_URL "https://blog.csdn.net/wanghualin033")
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_STANDARD 17)

file(GLOB NET_TEST_FILES main.cpp ../*.cpp ../epoll/*.cpp 
                        ../tcp/*.cpp ../udp/*.cpp)

add_executable(${PROJECT_NAME} ${NET_TEST_FILES})

target_link_libraries(${PROJECT_NAME} pthread)