
file(GLOB_RECURSE LSP_SOURCES *.cpp)
file(GLOB_RECURSE LSP_HEADERS *.h)

set(LSP_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR})

add_library(liblsp STATIC ${LSP_SOURCES})
SET_TARGET_PROPERTIES(liblsp PROPERTIES PREFIX "")

target_include_directories(liblsp PRIVATE ${LSP_INCLUDES} ${INCLUDES} ${rapidjson_source_dir}/include)

add_dependencies(liblsp rapidjson)
add_dependencies(liblsp libcommon)
