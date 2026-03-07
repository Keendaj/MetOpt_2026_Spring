cmake_minimum_required(VERSION 3.10)

project(LPSolver CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -O2)
endif()

add_executable(lp_solver 
    main.cpp 
    LinearProblemClass.cpp
)