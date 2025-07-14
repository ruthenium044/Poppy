cmake_minimum_required(VERSION 3.20)

project(Poppy)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)

# SDL settings
set(SDLIMAGE_AVIF OFF)
set(SDLIMAGE_BMP OFF)
set(SDLIMAGE_GIF OFF)
set(SDLIMAGE_JPG OFF)
set(SDLIMAGE_JXL OFF)
set(SDLIMAGE_LBM OFF)
set(SDLIMAGE_PCX OFF)
set(SDLIMAGE_PNM OFF)
set(SDLIMAGE_QOI OFF)
set(SDLIMAGE_SVG OFF)
set(SDLIMAGE_TGA OFF)
set(SDLIMAGE_TIF OFF)
set(SDLIMAGE_WEBP OFF)
set(SDLIMAGE_XCF OFF)
set(SDLIMAGE_XPM OFF)
set(SDLIMAGE_XV OFF)
set(SDLIMAGE_PNG ON)

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

# Fetch SDL3_image
FetchContent_Declare(
    SDL_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
    GIT_TAG prerelease-3.1.1
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(SDL SDL_image)

include_directories(${PROJECT_NAME} include)

file(GLOB_RECURSE HEADERS "src/*.h")
file(GLOB_RECURSE SOURCES_CPP "src/*.cpp")
file(GLOB_RECURSE SOURCES_C "src/*.c")

add_executable(${PROJECT_NAME} ${SOURCES_CPP} ${SOURCES_C} ${HEADERS}  "src/shader.h")

# Absolute path to the engine/dev resources!!
set(GL_RESOURCE_DIRECTORY_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE "GL_RESOURCE_DIRECTORY_PATH= \"${GL_RESOURCE_DIRECTORY_PATH}\"")

add_dependencies(${PROJECT_NAME} SDL3::SDL3 SDL3_image::SDL3_image)

target_link_libraries(${PROJECT_NAME} SDL3::SDL3 SDL3_image::SDL3_image opengl32)