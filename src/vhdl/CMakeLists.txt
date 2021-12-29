

# Do we have python v3+
find_package(Python3 REQUIRED)

set(NATSUKI ${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/natsuki.py)
set(VHDL_NODES ${CMAKE_CURRENT_SOURCE_DIR}/vhdl.nodes)
set(VHDL_NODES_H ${CMAKE_CURRENT_BINARY_DIR}/vhdl_nodes.h)
set(VHDL_NODES_CPP ${CMAKE_CURRENT_BINARY_DIR}/vhdl_nodes.cpp)
set(VHDL_NODES_DEBUG_H ${CMAKE_CURRENT_BINARY_DIR}/vhdl_nodes_debug.h)
set(VHDL_NODES_DEBUG_CPP ${CMAKE_CURRENT_BINARY_DIR}/vhdl_nodes_debug.cpp)

add_custom_command(
    COMMAND ${Python3_EXECUTABLE} ${NATSUKI} -nb ${VHDL_NODES} -o vhdl_nodes
    DEPENDS ${NATSUKI} ${VHDL_NODES}
    OUTPUT ${VHDL_NODES_H}
    COMMENT "[NATSUKI][vhdl_nodes] Building vhdl_nodes.h"
)

add_custom_command(
    COMMAND ${Python3_EXECUTABLE} ${NATSUKI} -nc ${VHDL_NODES} -o vhdl_nodes
    DEPENDS ${NATSUKI} ${VHDL_NODES}
    OUTPUT ${VHDL_NODES_CPP}
    COMMENT "[NATSUKI][vhdl_nodes] Building vhdl_nodes.cpp"
)

add_custom_command(
    COMMAND ${Python3_EXECUTABLE} ${NATSUKI} -db ${VHDL_NODES} -o vhdl_nodes
    DEPENDS ${NATSUKI} ${VHDL_NODES}
    OUTPUT ${VHDL_NODES_DEBUG_H}
    COMMENT "[NATSUKI][vhdl_nodes] Building vhdl_nodes_debug.h"
)

add_custom_command(
    COMMAND ${Python3_EXECUTABLE} ${NATSUKI} -dc ${VHDL_NODES} -o vhdl_nodes
    DEPENDS ${NATSUKI} ${VHDL_NODES}
    OUTPUT ${VHDL_NODES_DEBUG_CPP}
    COMMENT "[NATSUKI][vhdl_nodes] Building vhdl_nodes_debug.cpp"
)

add_custom_target(vhdl_nodes ALL DEPENDS ${VHDL_NODES_H} ${VHDL_NODES_CPP} ${VHDL_NODE_DEBUG_H} ${VHDL_NODES_DEBUG_CPP})

set(VHDL_SYNTAX ${CMAKE_CURRENT_SOURCE_DIR}/vhdl.syntax)
set(VHDL_SYNTAX_H ${CMAKE_CURRENT_BINARY_DIR}/vhdl_syntax.h)
set(VHDL_SYNTAX_CPP ${CMAKE_CURRENT_BINARY_DIR}/vhdl_syntax.cpp)
set(VHDL_SYNTAX_DEBUG_H ${CMAKE_CURRENT_BINARY_DIR}/vhdl_syntax_debug.h)
set(VHDL_SYNTAX_DEBUG_CPP ${CMAKE_CURRENT_BINARY_DIR}/vhdl_syntax_debug.cpp)

add_custom_command(
    COMMAND ${Python3_EXECUTABLE} ${NATSUKI} -nb ${VHDL_SYNTAX} -o vhdl_syntax
    DEPENDS ${NATSUKI} ${VHDL_SYNTAX}
    OUTPUT ${VHDL_SYNTAX_H}
    COMMENT "[NATSUKI][vhdl_syntax] Building vhdl_syntax.h"
)

add_custom_command(
    COMMAND ${Python3_EXECUTABLE} ${NATSUKI} -nc ${VHDL_SYNTAX} -o vhdl_syntax
    DEPENDS ${NATSUKI} ${VHDL_SYNTAX}
    OUTPUT ${VHDL_SYNTAX_CPP}
    COMMENT "[NATSUKI][vhdl_syntax] Building vhdl_syntax.cpp"
)

add_custom_command(
    COMMAND ${Python3_EXECUTABLE} ${NATSUKI} -db ${VHDL_SYNTAX} -o vhdl_syntax
    DEPENDS ${NATSUKI} ${VHDL_SYNTAX}
    OUTPUT ${VHDL_SYNTAX_DEBUG_H}
    COMMENT "[NATSUKI][vhdl_syntax] Building vhdl_syntax_debug.h"
)

add_custom_command(
    COMMAND ${Python3_EXECUTABLE} ${NATSUKI} -dc ${VHDL_SYNTAX} -o vhdl_syntax
    DEPENDS ${NATSUKI} ${VHDL_SYNTAX}
    OUTPUT ${VHDL_SYNTAX_DEBUG_CPP}
    COMMENT "[NATSUKI][vhdl_syntax] Building vhdl_syntax_debug.cpp"
)

add_custom_target(vhdl_syntax ALL DEPENDS
    ${VHDL_NODES_H} ${VHDL_NODES_CPP} ${VHDL_NODES_DEBUG_H} ${VHDL_NODES_DEBUG_CPP}
    ${VHDL_SYNTAX_H} ${VHDL_SYNTAX_CPP} ${VHDL_SYNTAX_DEBUG_H} ${VHDL_SYNTAX_DEBUG_CPP}
)

# ------------------------------------------------
# We then gather files that make the vhdl parser
# ------------------------------------------------

# ------------

file(GLOB_RECURSE VHDL_SOURCES *.cpp)
file(GLOB_RECURSE VHDL_HEADERS *.h)

set(VHDL_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_library(libvhdl STATIC ${VHDL_SOURCES} ${VHDL_NODES_CPP} ${VHDL_NODES_DEBUG_CPP} ${VHDL_SYNTAX_CPP} ${VHDL_SYNTAX_DEBUG_CPP})
set_target_properties(libvhdl PROPERTIES PREFIX "")

target_include_directories(libvhdl PUBLIC ${VHDL_INCLUDES} ${INCLUDES} ${rapidjson_source_dir}/include)
target_link_libraries(libvhdl libsqlite3)

add_dependencies(libvhdl rapidjson)
add_dependencies(libvhdl libcommon)
add_dependencies(libvhdl vhdl_nodes)
add_dependencies(libvhdl vhdl_syntax)
