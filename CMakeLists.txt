cmake_minimum_required(VERSION 3.21)
project(entsh)

add_executable(entsh WIN32
  src/main.cpp)

target_compile_definitions(entsh
  PUBLIC -DUNICODE=1
  PUBLIC -D_UNICODE=1)
set_property(TARGET entsh PROPERTY CXX_STANDARD 17)
