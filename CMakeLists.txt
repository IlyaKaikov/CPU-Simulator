cmake_minimum_required(VERSION 3.20)

project(cpu_simulator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(cpu_simulator
    src/cpu/CPU.cpp
    src/isa/Instruction.cpp
    src/memory/Memory.cpp
)

target_include_directories(cpu_simulator
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(cpu_simulator PRIVATE /W4 /permissive-)
else()
    target_compile_options(cpu_simulator PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(cpu_sim src/main.cpp)
target_link_libraries(cpu_sim PRIVATE cpu_simulator)

enable_testing()

add_executable(cpu_sim_tests tests/cpu_tests.cpp)
target_link_libraries(cpu_sim_tests PRIVATE cpu_simulator)

add_test(NAME cpu_sim_tests COMMAND cpu_sim_tests)
