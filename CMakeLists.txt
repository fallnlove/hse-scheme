cmake_minimum_required(VERSION 3.14)
project(scheme)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(scheme
    main.cpp
    src/tokenizer.cpp
    src/parser.cpp
    src/scheme.cpp
    src/object.cpp
    src/heap.cpp
)
