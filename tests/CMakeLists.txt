cmake_minimum_required(VERSION 2.8)

include_directories(subreg-tests
    "${CMAKE_SOURCE_DIR}/"
    "${CMAKE_SOURCE_DIR}/../"
)

add_executable(subreg-tests
    subreg-tests.c
    ../subreg.c
)
