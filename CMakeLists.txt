cmake_minimum_required(VERSION 3.12)
project(intercalacao-balanceada C)

set(CMAKE_C_STANDARD 11)

add_executable(intercalacao-balanceada main.c libs/tapes.h libs/tapes.c libs/render.h libs/render.c)