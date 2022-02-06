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

set(GLSLC_PATH /usr/bin/glslc)
set(SHSL_SHADER_SOURCE_DIRECTORY ${CMAKE_SOURCE_DIR}/shaders/src)
set(SHSL_SHADER_COMPILED_DIRECTORY ${CMAKE_SOURCE_DIR}/shaders/cmpld)

file(GLOB SHSL_SHADER_SOURCES ${SHSL_SHADER_SOURCE_DIRECTORY}/*.*)
foreach(file ${SHSL_SHADER_SOURCES})
    get_filename_component(FILENAME_WITHOUT_EXT ${file} NAME_WE)
    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
            COMMAND ${GLSLC_PATH} ${file} -o ${SHSL_SHADER_COMPILED_DIRECTORY}/${FILENAME_WITHOUT_EXT}.spv)
endforeach()