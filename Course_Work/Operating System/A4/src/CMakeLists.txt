cmake_minimum_required(VERSION 3.17)
project(source CUDA)

set(CMAKE_CUDA_STANDARD 11)

add_executable(source main.cu file_system.cu user_program.cu)

set_target_properties(
        source
        PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON)