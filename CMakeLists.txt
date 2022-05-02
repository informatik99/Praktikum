cmake_minimum_required(VERSION 3.10.2)
project(Praktikum C)

set(CMAKE_C_STANDARD 99)



add_executable(Praktikum main.c keyValStore.c keyValStore.h)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(Praktikum PRIVATE Threads::Threads)