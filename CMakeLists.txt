cmake_minimum_required(VERSION 3.20)

project(Poppy)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)

set(BUILD_SHARED_LIBS OFF)
set(SDL_SHARED FALSE)
set(SDL_STATIC TRUE)

# Download SDL3 from github
FetchContent_Declare(
    SDL
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.0
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(SDL)

include_directories(${PROJECT_NAME} include)

file(GLOB_RECURSE HEADERS "src/*.h")
file(GLOB_RECURSE SOURCES_CPP "src/*.cpp")
file(GLOB_RECURSE SOURCES_C "src/*.c")

add_executable(${PROJECT_NAME} ${SOURCES_CPP} ${SOURCES_C} ${HEADERS}  "src/shader.h")

# Absolute path to the engine/dev resources!!
set(GL_RESOURCE_DIRECTORY_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE "GL_RESOURCE_DIRECTORY_PATH= \"${GL_RESOURCE_DIRECTORY_PATH}\"")

add_dependencies(${PROJECT_NAME} SDL3::SDL3)

target_link_libraries(${PROJECT_NAME} SDL3::SDL3 opengl32)