cmake_minimum_required(VERSION 3.10)
project(AssignmentTracker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(assignment_tracker server.cpp)

# Platform-specific socket libraries
if(WIN32)
    target_link_libraries(assignment_tracker ws2_32)
endif()
