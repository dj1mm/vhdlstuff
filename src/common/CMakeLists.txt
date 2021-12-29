
file(GLOB_RECURSE COMMON_SOURCES *.cpp)
file(GLOB_RECURSE COMMON_HEADERS *.h)

set(COMMON_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR})

add_library(libcommon STATIC ${COMMON_SOURCES})
SET_TARGET_PROPERTIES(libcommon PROPERTIES PREFIX "")

target_include_directories(libcommon PUBLIC ${COMMON_INCLUDES} ${rapidjson_source_dir}/include)

add_dependencies(libcommon rapidjson)
