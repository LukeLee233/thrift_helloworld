# reference:  https://www.cnblogs.com/albizzia/p/10884320.html

cmake_minimum_required(VERSION 3.17)
project(thrift_helloworld)

set(CMAKE_CXX_STANDARD 14)


#Make sure gen-cpp files can be included
set(THRIFT_INC "$ENV{HOME}/opt/module/thrift/include")
set(THRIFT_LIB "$ENV{HOME}/opt/module/thrift/lib")

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "THRIFT_INC:  ${THRIFT_INC} ")
message(STATUS "THRIFT_LIB:  ${THRIFT_LIB} ")


include_directories("${CMAKE_SOURCE_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/gen-cpp")
include_directories("${THRIFT_INC}")
link_directories("${THRIFT_LIB}")

file(GLOB helloworld_SRC
        "gen-cpp/*.cpp"
        "gen-cpp/*.h"
        )

list(FILTER helloworld_SRC EXCLUDE REGEX ".*skeleton.cpp$")
message(STATUS "helloworld_SRC:  ${helloworld_SRC} ")

add_library(gencpp STATIC ${helloworld_SRC})
target_link_libraries(gencpp thrift)

set(THRIFT_COMPILER "$ENV{HOME}/opt/module/thrift/bin/thrift")
message(STATUS "THRIFT_COMPILER:  ${THRIFT_COMPILER} ")

add_executable(TutorialServer server.cpp)
target_link_libraries(TutorialServer gencpp)
if (ZLIB_FOUND)
    target_link_libraries(TutorialServer ${ZLIB_LIBRARIES})
endif ()

add_executable(TutorialClient client.cpp)
target_link_libraries(TutorialClient gencpp)
if (ZLIB_FOUND)
    target_link_libraries(TutorialClient ${ZLIB_LIBRARIES})
endif ()