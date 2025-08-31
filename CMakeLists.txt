cmake_minimum_required(VERSION 3.10)
set(TARGET_NAME Rasterizer)
project(TARGET_NAME)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin/Release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin/Debug)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(FINAL_OUTPUT_BIN_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})
else()
    set(FINAL_OUTPUT_BIN_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})
endif()


include(FetchContent)


add_library(glad_lib STATIC ${CMAKE_CURRENT_SOURCE_DIR}/Third/glad/src/glad.c)
target_include_directories(glad_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Third/glad/include)


# # Third-Dependencies: GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG "b35641f4a3c62aa86a0b3c983d163bc0fe36026d" # 3.4
    GIT_PROGRESS TRUE
)

FetchContent_GetProperties(glfw)

if(NOT glfw_POPULATED)
    FetchContent_Populate(glfw)

    set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
    set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
    set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
    set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")

    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})
endif()


# # Build

add_executable(${TARGET_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
)


# # Third-Dependencies: GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/icaven/glm
    GIT_TAG "dca38025fba63bb9284023e6de55f756b9e37cec"
    GIT_PROGRESS TRUE
)

FetchContent_GetProperties(glm)

if(NOT glm_POPULATED)
    FetchContent_Populate(glm)
    target_include_directories(${TARGET_NAME} PUBLIC ${glm_SOURCE_DIR}/)
endif()


target_include_directories(Rasterizer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Third/
    ${CMAKE_CURRENT_SOURCE_DIR}/
)

target_link_libraries(${TARGET_NAME} 
    glfw glad_lib
)

add_custom_target(copy_shaders ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/Shader
    ${FINAL_OUTPUT_BIN_PATH}/Shader
)