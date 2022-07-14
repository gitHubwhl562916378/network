cmake_minimum_required (VERSION 3.8)

project(GeelyNet
        LANGUAGES C CXX
        DESCRIPTION "network"
        HOMEPAGE_URL "https://do.geely.com/codingRoot/IMD-Public/GCComon")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/out)

if(UNIX)
file(GLOB SRC *.cpp udp/*.cpp tcp/*.cpp epoll/*.cpp)
else()
file(GLOB SRC
            INetHost.cpp
            udp/*.cpp
            iocp/*.cpp)
endif()

add_library(${PROJECT_NAME} SHARED ${SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION 1 SOVERSION 1)

if(UNIX)
target_link_libraries(${PROJECT_NAME} pthread)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_SYSTEM_NAME}/bin
                                LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_SYSTEM_NAME}/lib
                                ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_SYSTEM_NAME}/lib)

file(GLOB HEADERS ${CMAKE_SOURCE_DIR}/*.h)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/net)
file(GLOB HEADERS ${CMAKE_SOURCE_DIR}/iocp/*.h)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/net/iocp)
file(GLOB HEADERS ${CMAKE_SOURCE_DIR}/udp/*.h)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/net/udp)
file(GLOB HEADERS ${CMAKE_SOURCE_DIR}/tcp/*.h)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/net/tcp)
file(GLOB HEADERS ${CMAKE_SOURCE_DIR}/epoll/*.h)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/net/epoll)