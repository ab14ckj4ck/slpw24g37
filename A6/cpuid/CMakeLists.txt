cmake_minimum_required(VERSION 3.5)

project(A6_C
  LANGUAGES C ASM)


add_executable(A6_cpuid
  cpuid.c
  )
target_compile_options(A6_cpuid PRIVATE
  -m64 -O0 -Wall -Wextra -g)
