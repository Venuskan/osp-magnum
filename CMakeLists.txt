##
# Open Space Program
# Copyright © 2019-2020 Open Space Program Project
#
# MIT License
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
##

# Set CMake minimum version and CMake policy
CMAKE_MINIMUM_REQUIRED(VERSION 3.12.4)

# Enable link time optimization
CMAKE_POLICY(SET CMP0069 NEW)

# Set project name
PROJECT(OSP-MAGNUM CXX)

OPTION(OSP_BUILD_SANATIZER          "Build with the address sanatizer" OFF)
OPTION(OSP_WARNINGS_ARE_ERRORS      "Build with the flag -Werror" OFF)
OPTION(OSP_ENABLE_COMPILER_WARNINGS "Build with the majority of compiler warnings enabled" OFF)
OPTION(OSP_ENABLE_IWYU              "Build with warnings from IWYU turned on" OFF)
OPTION(OSP_ENABLE_CLANG_TIDY        "Build with warnings from clang-tidy turned on" OFF)
OPTION(OSP_USE_SYSTEM_SDL           "Build with SDL that you provide if turned on, compiles SDL if turned off. Off by default" OFF)

# Define target name
SET(TARGET_NAME OSP-MAGNUM)

# Set the explicit standard revision for C and C++ code.
SET(CMAKE_C_STANDARD 11)
SET(CMAKE_C_EXTENSIONS OFF)
SET(CMAKE_C_STANDARD_REQUIRED ON)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_EXTENSIONS OFF)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# If the environment has these set, pull them into proper cariables.
SET(CLANG_COMPILE_FLAGS "${CLANG_COMPILE_FLAGS}")
SET(GCC_COMPILE_FLAGS   "${GCC_COMPILE_FLAGS}")

# Compiler warnings can help find problems in code.
IF(OSP_ENABLE_COMPILER_WARNINGS)
  # Enable additional warnings.
  # -fno-omit-frame-pointer significantly helps with debugging of the application.
  STRING(APPEND CMAKE_C_FLAGS   " -Wall -Wextra -fno-omit-frame-pointer")
  STRING(APPEND CMAKE_CXX_FLAGS " -Wall -Wextra -fno-omit-frame-pointer")

  # These are hand selected warnings that are enabled by -Wextra that tend to be non-issues, so disable them explicitly for sake of avoiding build noise.
  STRING(APPEND CLANG_COMPILE_FLAGS " -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-undefined-func-template -Wno-unused-template")
  STRING(APPEND CLANG_COMPILE_FLAGS " -Wno-deprecated -Wno-documentation -Wno-documentation-deprecated-sync -Wno-documentation-unknown-command -Wno-abstract-vbase-init")
  STRING(APPEND CLANG_COMPILE_FLAGS " -Wno-switch-enum -Wno-covered-switch-default -Wno-missing-prototypes -Wno-sign-conversion -Wno-float-conversion -Wno-shorten-64-to-32")
  STRING(APPEND CLANG_COMPILE_FLAGS " -Wno-shadow -Wno-shadow-field-in-constructor -Wshadow-uncaptured-local -Wno-inconsistent-missing-destructor-override")
  STRING(APPEND CLANG_COMPILE_FLAGS " -Wno-global-constructors -Wno-exit-time-destructors  -Wno-weak-vtables -Wno-undef -Wno-disabled-macro-expansion")

  # This relatively rare warning can be emitted because of third party code, and is basically not our problem.
  STRING(APPEND GCC_COMPILE_FLAGS " -Wno-psabi")
ENDIF() # OSP_ENABLE_COMPILER_EARNINGS

# Compiler warnings can help find problems in code.
IF(OSP_WARNINGS_ARE_ERRORS)
  STRING(APPEND CMAKE_C_FLAGS   " -Werror")
  STRING(APPEND CMAKE_CXX_FLAGS " -Werror")
ENDIF() # OSP_WARNINGS_ARE_ERRORS

# The sanatizers provide compile time code instrumentation that drastically improve the ability of programmars to find bugs.
IF(OSP_BUILD_SANATIZER)
  STRING(APPEND GCC_COMPILE_FLAGS " -fstack-protector-all -fsanitize=address,bounds,enum,leak,pointer-compare,pointer-subtract -fsanitize-address-use-after-scope")
ENDIF() # OSP_BUILD_SANATIZER

IF(CMAKE_C_COMPILER_ID MATCHES "Clang")
  STRING(APPEND CMAKE_C_FLAGS " ${CLANG_COMPILE_FLAGS}")
ENDIF()
IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  STRING(APPEND CMAKE_CXX_FLAGS " ${CLANG_COMPILE_FLAGS}")
ENDIF()

IF(CMAKE_C_COMPILER_ID MATCHES "GNU")
  STRING(APPEND CMAKE_C_FLAGS " ${GCC_COMPILE_FLAGS}")
ENDIF()
IF(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  STRING(APPEND CMAKE_CXX_FLAGS " ${GCC_COMPILE_FLAGS}")
ENDIF()

# In debug builds, disable optimizations (just in case they were enabled by a previous flag, or env variable)
# and mark the debug symbols to be compressed, which can be a huge saving in terms of file size.
IF(MSVC)
ELSE()
  STRING(APPEND CMAKE_C_FLAGS_DEBUG   " -gz -O0")
  STRING(APPEND CMAKE_CXX_FLAGS_DEBUG " -gz -O0")
ENDIF()

IF(CMAKE_BUILD_TYPE MATCHES "Debug")
# Don't enable LTO in debug builds.
ELSE()
  INCLUDE(CheckIPOSupported)
  CHECK_IPO_SUPPORTED(RESULT lto_supported OUTPUT error)
  IF( lto_supported )
    MESSAGE(STATUS "IPO / LTO enabled")
    SET(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
  ELSE()
    MESSAGE(STATUS "IPO / LTO not supported: <${error}>")
  ENDIF()
ENDIF()

# Force Runtime output directory into build/bin folder; force multiprocessor compilation for all projects
IF(MSVC)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG> CACHE PATH "")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/lib CACHE PATH "")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/lib CACHE PATH "")
  add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)
ENDIF()

STRING(PREPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules/")

# Process subdirectory full of third-party libraries
ADD_SUBDIRECTORY(3rdparty)

# Also process the src subdirectory, which describes the source code for the project.
ADD_SUBDIRECTORY(src)

# Unit Tests
ADD_SUBDIRECTORY(test)
