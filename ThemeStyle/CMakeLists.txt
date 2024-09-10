cmake_minimum_required(VERSION 3.5)
project(StylerManager)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add ImGui source files
set(IMGUI_DIR "imgui")
file(GLOB IMGUI_SOURCES ${IMGUI_DIR}/*.cpp)

# Find OpenGL package
find_package(OpenGL REQUIRED)

# Add GLFW library
find_package(glfw3 REQUIRED)

# Include directories
include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${IMGUI_DIR})
include_directories("rapidjson")
#include_directories("leif/include")  # Include leif header files

# Add glad files to your project
set(GLAD_DIR "glad")
set(GLAD_SOURCES ${GLAD_DIR}/glad.c)
set(CMAKE_BUILD_TYPE Debug)
include_directories(${GLAD_DIR})


# Link leif library
#link_directories("leif/lib")

# Find libnotify package
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBNOTIFY REQUIRED libnotify)

# Include GLib headers
include_directories(${LIBNOTIFY_INCLUDE_DIRS})

# Add executable
add_executable(StylerManager main.cpp apply_system_theme.hpp apply_system_theme.cpp file_manager.hpp ${IMGUI_SOURCES} ${GLAD_SOURCES})

# Link libraries
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
target_link_libraries(StylerManager ${OPENGL_LIBRARIES} clipboard glfw ${LIBNOTIFY_LIBRARIES})

find_package(OpenSSL REQUIRED)
target_link_libraries(StylerManager OpenSSL::SSL)
