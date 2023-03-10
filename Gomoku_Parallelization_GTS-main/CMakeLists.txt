#
# blupig
# Copyright (C) 2016-2017 Yunzhu Li
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

cmake_minimum_required(VERSION 2.8)
project(blupig)

set(CMAKE_CXX_COMPILER "/usr/bin/g++")
#output compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Enable C++ 11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3 -ffast-math")

# Enable parallel flags
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -mavx2 ")

# Include header directories
include_directories("include")
include_directories("tests")

# Add source files
file(GLOB_RECURSE SRC "src/*.cc")
file(GLOB_RECURSE SRC_TEST "tests/*.cc")

# Set default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

# Main executable
add_executable(gomoku ${SRC})

# Profiling executable
if (ENABLE_PROFILING)
    set(CMAKE_BUILD_TYPE Debug)
    add_executable(gomoku_prof ${SRC})
    set_target_properties(gomoku_prof PROPERTIES COMPILE_FLAGS "-pg")
    set_target_properties(gomoku_prof PROPERTIES LINK_FLAGS "-pg")
endif()

# Test executable
if (ENABLE_TESTING)
    add_executable(gomoku_test ${SRC} ${SRC_TEST})
    set_target_properties(gomoku_test PROPERTIES COMPILE_FLAGS "-D BLUPIG_TEST")
    find_package(Threads)
    target_link_libraries(gomoku_test ${CMAKE_THREAD_LIBS_INIT})
endif()

# Allow installing using 'make install'
install(TARGETS gomoku DESTINATION bin)
