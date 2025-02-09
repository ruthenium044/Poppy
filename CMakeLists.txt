cmake_minimum_required(VERSION 3.20)

project(OpEngl)

include(FetchContent)

set(CMAKE_CXX_STANDARD 17)

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

add_executable(OpEngl ${SOURCES_CPP} ${SOURCES_C} ${HEADERS})

add_dependencies(OpEngl SDL3::SDL3)

target_link_libraries(OpEngl SDL3::SDL3 opengl32)