cmake_minimum_required(VERSION 3.5)

project(Network LANGUAGES CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(LIB_NAME Network)

# Use pkg-config to find DBus and GLib
find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS REQUIRED dbus-1)
pkg_check_modules(DBUS_GLIB REQUIRED dbus-glib-1)
pkg_check_modules(GLIB REQUIRED glib-2.0)

# Specify header and source files
file(GLOB LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Ensure there are source files
if (NOT SOURCES)
    message(FATAL_ERROR "No source files found in ${CMAKE_CURRENT_SOURCE_DIR}/src/")
endif()

# Print the sources for debugging
message(STATUS "Source files: ${SOURCES}")

# Add the library
add_library(${LIB_NAME} SHARED ${SOURCES})

# Include directories for the library
target_include_directories(${LIB_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${DBUS_INCLUDE_DIRS}
    ${DBUS_GLIB_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}  # Add GLib include directories
)

# Link D-Bus, D-Bus GLib, and GLib to the library
target_link_libraries(${LIB_NAME} PUBLIC ${DBUS_LIBRARIES} ${DBUS_GLIB_LIBRARIES} ${GLIB_LIBRARIES})
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -rdynamic")
