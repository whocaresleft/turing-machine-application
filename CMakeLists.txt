cmake_minimum_required(VERSION 3.31)
project(mdt)

set(CMAKE_CXX_STANDARD 20)

add_executable(mdt main.cpp
        Header/definitions.h
        Header/couple.h
        Header/alphabet.h
        Header/Tape.h
        Header/turing_machine.h
        Header/computation.h
        Header/helper.h)
