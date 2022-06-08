cmake_minimum_required(VERSION 3.7.2)
project(BSP C)

set(CMAKE_C_STANDARD 99)

add_executable(BSP main.c main.h sub.c sub.h keyValStore.c keyValStore.h subscription.c subscription.h)
