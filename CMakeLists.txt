cmake_minimum_required(VERSION 3.1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

add_executable(
  productor
  src/productor.c
)

add_executable(
  consumidor
  src/consumidor.c
)
