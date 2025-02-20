cmake_minimum_required ( VERSION 3.5 )

project ( OpenXcom )

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

# Set project to C++20 standard
set (CMAKE_CXX_STANDARD 20)

# Require the specified C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory ( src )