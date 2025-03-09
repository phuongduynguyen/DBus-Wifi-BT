cmake_minimum_required(VERSION 3.5)

project(Network LANGUAGES CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(LIB_NAME Network)

add_compile_options(-fPIC)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS REQUIRED dbus-1)
pkg_check_modules(DBUS_GLIB REQUIRED dbus-glib-1)
pkg_check_modules(GLIB REQUIRED glib-2.0)

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

if (NOT SOURCES)
    message(FATAL_ERROR "No source files found in ${CMAKE_CURRENT_SOURCE_DIR}/src/")
endif()

message(STATUS "Source files: ${SOURCES}")
message(STATUS "DBus include dirs: ${DBUS_INCLUDE_DIRS}")

add_library(${LIB_NAME} SHARED ${SOURCES})

target_include_directories(${LIB_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/public>
        $<INSTALL_INTERFACE:include>
        ${DBUS_INCLUDE_DIRS}
        ${DBUS_GLIB_INCLUDE_DIRS}
        ${GLIB_INCLUDE_DIRS}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/private
)

set_target_properties(${LIB_NAME} PROPERTIES
    PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include/public/NetworkProvider.h
)

target_link_libraries(${LIB_NAME} PUBLIC 
    ${DBUS_LIBRARIES} 
    ${DBUS_GLIB_LIBRARIES} 
    ${GLIB_LIBRARIES}
)

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -rdynamic")

install(TARGETS ${LIB_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include
)