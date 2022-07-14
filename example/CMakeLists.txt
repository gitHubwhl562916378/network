cmake_minimum_required (VERSION 3.8)

project(example
        LANGUAGES C CXX
        DESCRIPTION "network"
        HOMEPAGE_URL "https://do.geely.com/codingRoot/IMD-Public/GCComon")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE "Debug")
set(NET_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/../out)

if(UNIX)
file(GLOB SRC sample_gtest.cpp)
else()
file(GLOB SRC sample.cpp)
endif()

include_directories(${NET_INSTALL_PREFIX}/include/net)
link_directories(${NET_INSTALL_PREFIX}/${CMAKE_SYSTEM_NAME}/lib)

add_executable(${PROJECT_NAME} ${SRC})
if(WIN32)
add_custom_command(TARGET ${PROJECT_NAME}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${NET_INSTALL_PREFIX}/${CMAKE_SYSTEM_NAME}/bin/GeelyNet.dll .)
endif()

if(UNIX)
target_link_libraries(${PROJECT_NAME} GeelyNet gtest gtest_main pthread)
else()
target_link_libraries(${PROJECT_NAME} GeelyNet)
endif()