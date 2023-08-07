
cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(vhdlstuff)

enable_testing()

add_subdirectory(src)

# ------------------------------------------------
# We then include the common tests
# ------------------------------------------------

add_subdirectory(tests)

