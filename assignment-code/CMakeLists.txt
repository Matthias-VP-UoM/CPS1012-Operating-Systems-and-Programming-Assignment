cmake_minimum_required(VERSION 3.15)
project(os-assignment C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused)

include_directories(.)

# Task 1
add_executable(q1a task1/1a.c)
add_executable(q1b task1/1b.c)
add_executable(q1c task1/1c.c)
add_executable(q1d task1/1d.c)
add_executable(q1e task1/1e.c)
add_executable(task1prog task1/task1.c)

# Task 2
add_executable(q2a task2/2a.c)
add_executable(q2b task2/2b.c task2/task2_builtin_functions.c)
add_executable(task2prog task2/task2.c task2/task2_builtin_functions.c)

# Task 3 - built from Tasks 1 and 2
add_executable(q3a task3/3a.c)
add_executable(q3b task3/3b.c)
add_executable(q3c task3/3c.c)
add_executable(q3d task3/3d.c)
add_executable(task3prog task3/task3.c)

# Task 4 - built from Tasks 1, 2 and 3
add_executable(q4a task4/4a.c)
add_executable(q4b task4/4b.c)
add_executable(q4c task4/4c.c)
add_executable(task4prog task4/task4.c)

# Main program - task4.c contains the final program
add_executable(mainprog maintish.c task4/task4.c)
