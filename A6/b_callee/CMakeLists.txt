cmake_minimum_required(VERSION 3.5)

project(A6_B
  LANGUAGES C ASM)

## B) CALLEE

add_executable(A6_sysv_callee
  main.c
  sysv_abi.S)
target_compile_options(A6_sysv_callee PRIVATE
  -m64 -O0 -Wall -Wextra -g)
