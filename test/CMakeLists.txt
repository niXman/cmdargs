cmake_minimum_required(VERSION 3.1)

project(cmdargs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_ASAN "Enable AddressSanitizer")
option(TEST_MAX_OPTIONS "Enable test for max options number")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

if(ENABLE_ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
endif()

if(TEST_MAX_OPTIONS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTEST_MAX_OPTIONS")
endif()

message("G++ CMDLINE:${CMAKE_CXX_FLAGS}")

add_definitions(
    -UNDEBUG
)

include_directories(
    ../include
)

set(SOURCES
    ../include/cmdargs/cmdargs.hpp
    main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(
    ${PROJECT_NAME}
    pthread
)
