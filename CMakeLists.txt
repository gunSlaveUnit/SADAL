cmake_minimum_required(VERSION 3.21)
project(SADAL)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(${PROJECT_NAME} main.cpp engine/engine.cpp engine/engine.h engine/vertex.cpp engine/vertex.h engine/transformation.h engine/external/stb_image.h engine/external/stb_image.cpp engine/external/tiny_obj_loader.h engine/external/tiny_obj_loader.cpp)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

if(UNIX)
    find_package(glfw3 REQUIRED FATAL_ERROR)
    target_link_libraries(${PROJECT_NAME} glfw)
endif(UNIX)

if(WIN32)
    set(glm_DIR C:/GLM/cmake/glm)

    add_subdirectory(GLFW)
    target_link_libraries(${PROJECT_NAME} glfw)
    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME} OpenGL::GL)
endif(WIN32)

find_package(glm REQUIRED FATAL_ERROR)
include_directories(${GLM_INCLUDE_DIRS})
link_directories(${GLM_LIBRARY_DIRS})

find_package(Vulkan REQUIRED FATAL_ERROR)
if (VULKAN_FOUND)
    find_package(Vulkan REQUIRED)
    target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)
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