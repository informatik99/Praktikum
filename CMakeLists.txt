cmake_minimum_required(VERSION 3.10)
project(testServer C)

set(CMAKE_C_STANDARD 99)

add_executable(Praktikum main.c store.c store.h interaction.h interaction.c sub.h sub.c shared.h shared.c constants.h)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(Praktikum PRIVATE Threads::Threads)