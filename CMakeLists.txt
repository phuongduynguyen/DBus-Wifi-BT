cmake_minimum_required(VERSION 3.5)

project(NetworkRun LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
add_subdirectory(Lib)
set(LIB_NAME Network)


include_directories(/home/duynp/C++/DBus/Lib/include)

add_executable(run ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
set_property(TARGET run PROPERTY CXX_STANDARD 17)
target_link_libraries(run ${LIB_NAME})

include(GNUInstallDirs)

install(TARGETS Network run
        LIBRARY 
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME 
            DESTINATION ${CMAKE_INSTALL_BINDIR})

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/deploy)
set_target_properties(run PROPERTIES LINK_FLAGS "-rdynamic")
