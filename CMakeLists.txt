# MIT License
#
# Copyright (c) 1994–2026 Lua.org, PUC-Rio.
# Copyright (c) 2023-2026 The OneLuaPro project authors.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in 
# the Software without restriction, including without limitation the rights to 
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all 
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# ------------------------------------------------------------------------------
# CMake for Lua
# ------------------------------------------------------------------------------
# Setup with Visual Studio 17 2022 generator for x64
# ------------------------------------------------------------------------------
# Visual Studio is a mutlti-configuration generator
# https://stackoverflow.com/questions/24460486/
#
# Basic instructions for out-of-source build with MSVC
# ----------------------------------------------------
# mkdir build && cd build
# cmake .. -G "Visual Studio 17 2022" -A x64
# cmake --build . --config Release
# cmake --install . --config Release
# 
# Available architectures (-A ...) are: Win32, x64, ARM, ARM64
#
# Basic instructions for out-of-source build with IntelLLVM
# ---------------------------------------------------------
# mkdir build && cd build
# cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=icx -DCMAKE_CXX_COMPILER=icx
# cmake --build . [--verbose]
# cmake --install .

# ------------------------------------------------------------------------------
# General definitions
cmake_minimum_required(VERSION 3.23 FATAL_ERROR)
# Extract current Lua version from lua.h. 5.4 spells the components as strings
# (LUA_VERSION_MAJOR "5"), 5.5 as bare numbers (LUA_VERSION_MAJOR_N 5), so accept
# both: this repository swaps Lua releases in and out, and matching only one form
# is what left LUA_VERSION empty after the 5.5 -> 5.4.8 swap - which then showed
# up as an install prefix of C:/Apps/lua-.. and a module directory of lib/lua/.
# Fail loudly instead of carrying an empty version through the build.
file(READ "${CMAKE_SOURCE_DIR}/src/lua.h" _header)
foreach(_v MAJOR MINOR RELEASE)
  if(_header MATCHES "#define[ \t]+LUA_VERSION_${_v}[ \t]+\"([0-9]+)\"")
    set(LUA_VERSION_${_v} "${CMAKE_MATCH_1}")
  elseif(_header MATCHES "#define[ \t]+LUA_VERSION_${_v}_N[ \t]+([0-9]+)")
    set(LUA_VERSION_${_v} "${CMAKE_MATCH_1}")
  else()
    message(FATAL_ERROR "Cannot read LUA_VERSION_${_v} from src/lua.h")
  endif()
endforeach()
set(LUA_VERSION "${LUA_VERSION_MAJOR}.${LUA_VERSION_MINOR}.${LUA_VERSION_RELEASE}")

# ------------------------------------------------------------------------------
# Project defintion
project(lua
  LANGUAGES C
  VERSION ${LUA_VERSION}
)

# ------------------------------------------------------------------------------
# Installation prefix directory - defaults to C:\Apps\lua-X.Y.Z
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  if(WIN32)
    set(CMAKE_INSTALL_PREFIX "C:/Apps/lua-${LUA_VERSION}" CACHE STRING
      "Installation Prefix" FORCE)
  else()
    message(FATAL_ERROR "Not yet implemented.")
  endif()
endif()

# Lua is completely ANSI-C (https://www.lua.org/faq.html#1.1)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------------------
# Other settings
set(CMAKE_VERBOSE_MAKEFILE OFF)
if(NOT CMAKE_BUILD_TYPE)
  # Visual Studio generators irgore this setting
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
# create valiable with upper-case content out of CMAKE_BUILD_TYPE
string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UC)

# ------------------------------------------------------------------------------
# Setup GNU-alike installatin directories
include (GNUInstallDirs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
set(INSTALL_DIR
  ${CMAKE_INSTALL_PREFIX} CACHE PATH "Install base dir for package config.")
set(INSTALL_LIBDIR
  ${CMAKE_INSTALL_LIBDIR} CACHE PATH "Installation directory for libraries")
set(INSTALL_BINDIR
  ${CMAKE_INSTALL_BINDIR} CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDEDIR
  ${CMAKE_INSTALL_INCLUDEDIR} CACHE PATH "Installation directory for header files")
set(INSTALL_DOCDIR
  ${CMAKE_INSTALL_DOCDIR} CACHE PATH "Installation directory for documentation")
set(INSTALL_MANDIR
  ${CMAKE_INSTALL_MANDIR} CACHE PATH "Installation directory for manpages")
set(DEF_INSTALL_CMAKEDIR share/cmake/lib${PROJECT_NAME})
set(INSTALL_CMAKEDIR
  ${DEF_INSTALL_CMAKEDIR} CACHE PATH "Installation directory for CMake files")
# Lua-specific installation dirs
set(INSTALL_TOP_CDIR
  ${INSTALL_LIBDIR}/lua/${LUA_VERSION_MAJOR}.${LUA_VERSION_MINOR})

# ------------------------------------------------------------------------------
# Report to user
message(STATUS "System name: ${CMAKE_SYSTEM_NAME}")
message(STATUS "System version: ${CMAKE_SYSTEM_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C compiler flags: ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE_UC}}")
message(STATUS "Installation prefix: ${CMAKE_INSTALL_PREFIX}")
foreach(p BIN INCLUDE LIB DOC MAN CMAKE)
  file(TO_NATIVE_PATH ${CMAKE_INSTALL_PREFIX}/${INSTALL_${p}DIR} _path)
  message(STATUS "Installing ${p} components to ${_path}")
  unset(_path)
endforeach()

# ------------------------------------------------------------------------------
# Dive into subdirs
add_subdirectory(src)
add_subdirectory(doc)
