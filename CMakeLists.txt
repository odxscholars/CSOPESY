cmake_minimum_required(VERSION 3.29)
project(ownImplementation)

set(CMAKE_CXX_STANDARD 20)

add_executable(ownImplementation main.cpp
        headers/Console.h
        Console.cpp
        Config.cpp
        headers/Config.h
        Process.cpp
        headers/Process.h
        headers/Scheduler.h
        Scheduler.cpp
        headers/MemoryManager.h
        MemoryManager.cpp)
