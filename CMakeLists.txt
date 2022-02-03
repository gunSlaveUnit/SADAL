cmake_minimum_required(VERSION 3.21)
project(ShatteredSouls)

set(CMAKE_CXX_STANDARD 20)

add_executable(ShatteredSouls main.cpp engine.cpp engine.h)

find_package(glm REQUIRED FATAL_ERROR)
include_directories(${GLM_INCLUDE_DIRS})
link_directories(${GLM_LIBRARY_DIRS})

find_package(glfw3 REQUIRED FATAL_ERROR)
target_link_libraries(ShatteredSouls glfw)

find_package(Vulkan REQUIRED FATAL_ERROR)
if (VULKAN_FOUND)
    target_include_directories (ShatteredSouls PUBLIC ${VULKAN_INCLUDE_DIR})
    target_link_libraries (ShatteredSouls ${Vulkan_LIBRARY})
endif()
