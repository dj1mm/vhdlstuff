
#
# Again, this file was shamefully copied from:
# https://raw.githubusercontent.com/kaizouman/gmake-cmake-example/master/test/CMakeLists.txt
#

# Do we have python v3+
find_package(Python3 REQUIRED)

set(NATSUKI ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/natsuki.py)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

find_package(Threads REQUIRED)

# Enable ExternalProject CMake module
include(ExternalProject)

# Download and install Catch2
ExternalProject_Add(
    catch2
    URL https://github.com/catchorg/catch2/archive/v2.x.zip
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/catch2
    INSTALL_COMMAND "" # Disable install step
)

# Get Catch2 source and binary directories from CMake project
ExternalProject_Get_Property(catch2 source_dir binary_dir)

include_directories(${source_dir}/single_include)

file(GLOB SOURCES *.cpp)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

# ------------------------------------------------
# Now generate the executable
# ------------------------------------------------

function(generate_natsuki_output_products input)
    add_custom_command(
        COMMAND ${Python3_EXECUTABLE} ${NATSUKI} -nb ${CMAKE_CURRENT_SOURCE_DIR}/${input}.nodes -o ${CMAKE_CURRENT_BINARY_DIR}/${input}
        DEPENDS ${NATSUKI} ${CMAKE_CURRENT_SOURCE_DIR}/${input}.nodes
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${input}.h
        COMMENT "[NATSUKI][${input}] Building ${input}.h"
    )
    add_custom_command(
        COMMAND ${Python3_EXECUTABLE} ${NATSUKI} -nc ${CMAKE_CURRENT_SOURCE_DIR}/${input}.nodes -o ${CMAKE_CURRENT_BINARY_DIR}/${input}
        DEPENDS ${NATSUKI} ${CMAKE_CURRENT_SOURCE_DIR}/${input}.nodes
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${input}.cpp
        COMMENT "[NATSUKI][${input}] Building ${input}.cpp"
    )
    add_custom_command(
        COMMAND ${Python3_EXECUTABLE} ${NATSUKI} -db ${CMAKE_CURRENT_SOURCE_DIR}/${input}.nodes -o ${CMAKE_CURRENT_BINARY_DIR}/${input}
        DEPENDS ${NATSUKI} ${CMAKE_CURRENT_SOURCE_DIR}/${input}.nodes
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${input}_debug.h
        COMMENT "[NATSUKI][${input}] Building ${input}_debug.h"
    )
    add_custom_command(
        COMMAND ${Python3_EXECUTABLE} ${NATSUKI} -dc ${CMAKE_CURRENT_SOURCE_DIR}/${input}.nodes -o ${CMAKE_CURRENT_BINARY_DIR}/${input}
        DEPENDS ${NATSUKI} ${CMAKE_CURRENT_SOURCE_DIR}/${input}.nodes
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${input}_debug.cpp
        COMMENT "[NATSUKI][${input}] Building ${input}_debug.cpp"
    )

    target_sources(teststuff PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/${input}.cpp ${CMAKE_CURRENT_BINARY_DIR}/${input}_debug.cpp)
    add_custom_target(${input} ALL DEPENDS ${input}.h ${input}.cpp ${input}_debug.h ${input}_debug.cpp)
    add_dependencies(teststuff ${input})
endfunction(generate_natsuki_output_products)

add_definitions(-DRAPIDJSON_HAS_STDSTRING)

add_executable(teststuff ${SOURCES})

generate_natsuki_output_products(value_natsuki_node_test)
generate_natsuki_output_products(value_natsuki_enum_test)
generate_natsuki_output_products(value_custom_obj_test)
generate_natsuki_output_products(value_custom_ptr_test)
generate_natsuki_output_products(optional_natsuki_node_test)
generate_natsuki_output_products(optional_natsuki_enum_test)
generate_natsuki_output_products(optional_custom_obj_test)
generate_natsuki_output_products(optional_custom_ptr_test)
generate_natsuki_output_products(array_natsuki_node_test)
generate_natsuki_output_products(array_natsuki_enum_test)
generate_natsuki_output_products(array_custom_obj_test)
generate_natsuki_output_products(array_custom_ptr_test)
generate_natsuki_output_products(map_natsuki_node_test)
generate_natsuki_output_products(map_natsuki_enum_test)
generate_natsuki_output_products(map_custom_obj_test)
generate_natsuki_output_products(map_custom_ptr_test)
generate_natsuki_output_products(node_inheritance_test)
generate_natsuki_output_products(referenced_natsuki_node_test)
generate_natsuki_output_products(conditionally_owned_things_test)

add_dependencies(teststuff catch2)

target_link_libraries(teststuff libvhdl liblsp libcommon libbigint libfmt)
target_include_directories(teststuff PRIVATE ${VHDL_INCLUDES})

if(LINUX)
    target_link_libraries(teststuff -lpthread -ldl -lstdc++fs)
endif()

add_test(NAME teststuff COMMAND teststuff)
