cmake_minimum_required(VERSION 2.8)
project(justargs)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra") # -fsanitize=address

add_definitions(
    -UNDEBUG
)

include_directories(
    ./include
)

set(SOURCES
    include/justargs/justargs.hpp
    main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(
    ${PROJECT_NAME}
    pthread
)
