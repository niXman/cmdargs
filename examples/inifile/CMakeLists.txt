cmake_minimum_required(VERSION 2.8)
project(inifile)

set (CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra") # -fsanitize=address

if( CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_DEBUG -std=c++14 -Wall -Wextra")
endif()

add_definitions(
    -UNDEBUG
)

include_directories(
    ../../include
)

set(SOURCES
    ../../include/justargs/justargs.hpp
    main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(
    ${PROJECT_NAME}
    pthread
)
