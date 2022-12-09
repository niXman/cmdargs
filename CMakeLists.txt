cmake_minimum_required(VERSION 2.8)

project(cmdargs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra") # -fsanitize=address

add_definitions(
    -UNDEBUG
)

include_directories(
    ./include
)

set(SOURCES
    include/cmdargs/cmdargs.hpp
    main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(
    ${PROJECT_NAME}
    pthread
)
