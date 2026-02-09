cmake_minimum_required(VERSION 3.5)

project(A6_A
  LANGUAGES C ASM)

## A) CALLER

add_executable(A6_sysv_caller
  caller.c
  call_me.c)
target_compile_options(A6_sysv_caller PRIVATE
  -m64 -O0 -Wall -Wextra -mno-red-zone -g)
