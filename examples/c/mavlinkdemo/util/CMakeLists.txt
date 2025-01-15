cmake_minimum_required(VERSION 3.13)
project(utils)

# 设置 C 标准
set(CMAKE_C_STANDARD 11)
# 设置源文件目录
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=default")
message(STATUS "SRC_DIR: ${SRC_DIR}")
message(STATUS "out: ${CMAKE_CURRENT_SOURCE_DIR}/../out")

# 查找所有 C 源文件
file(GLOB MAVLINK_SOURCES ${SRC_DIR}/*.c)
message(STATUS "MAVLINK_SOURCES: ${MAVLINK_SOURCES}")

# 创建共享库
add_library(utils SHARED ${MAVLINK_SOURCES})

set_target_properties(utils PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out)

