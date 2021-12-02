cmake_minimum_required(VERSION 3.17)
project(CSC3150_Assignment3_CUDA CUDA)

set(CMAKE_CUDA_STANDARD 14)

add_executable(CSC3150_Assignment3_CUDA main.cu virtual_memory.cu virtual_memory.cuh user_program.cu)

set_target_properties(
        CSC3150_Assignment3_CUDA
        PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON)