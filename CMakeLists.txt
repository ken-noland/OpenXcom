cmake_minimum_required ( VERSION 3.1 )

project ( OpenXcom )

# Set project to C++20 standard
set (CMAKE_CXX_STANDARD 20)

# Require the specified C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_subdirectory ( docs )
add_subdirectory ( src )