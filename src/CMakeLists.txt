find_package(Python3 REQUIRED)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()


# Download and install args
include(FetchContent)
FetchContent_Declare(
    args
    GIT_REPOSITORY https://github.com/Taywee/args
)
FetchContent_MakeAvailable(args)

# ------------

include(FetchContent)

# Download and install yaml-cpp
# https://github.com/jbeder/yaml-cpp/issues/1136
# https://github.com/jbeder/yaml-cpp/issues/774#issuecomment-737155168
FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp
    GIT_TAG 98acc5a8874faab28b82c28936f4b400b389f5d6
)
FetchContent_MakeAvailable(yaml-cpp)

# ------------

include(ExternalProject)

# Download and install rapidjson
ExternalProject_Add(
    rapidjson
    URL https://github.com/Tencent/rapidjson/archive/master.zip
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/rapidjson
    INSTALL_COMMAND "" # Disable install step
)

# Get rapidjson source and binary directories from CMake project
ExternalProject_Get_Property(rapidjson source_dir binary_dir)
set(rapidjson_source_dir ${source_dir})
set(rapidjson_binary_dir ${binary_dir})

# There is no need to compile rapidjson, as it is all .h

add_definitions(-DRAPIDJSON_HAS_STDSTRING)


set(INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "include folder")

# ------------

include(FetchContent)
FetchContent_Declare(
  slang
  GIT_REPOSITORY https://github.com/MikePopoloski/slang
  GIT_TAG v3.0
  GIT_SHALLOW ON)
FetchContent_MakeAvailable(slang)

# ------------


add_subdirectory(bigint)
add_subdirectory(common)
add_subdirectory(fmt)
add_subdirectory(sqlite3)
add_subdirectory(lsp)
add_subdirectory(vhdl)

add_custom_command(
    COMMAND ""
    OUTPUT always_run
    COMMENT "[VERSION][CHECK] Check repository"
)

add_custom_command(
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/git_helper.py
    DEPENDS always_run ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/git_helper.py
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
    COMMENT "[VERSION][APPLY] Apply repository changes if available"
)

file(GLOB MAINS *.cpp)
file(GLOB_RECURSE THINGS_SOURCES things/*.cpp)
file(GLOB_RECURSE THINGS_HEADERS things/*.h)

file(GLOB_RECURSE SV_SOURCES sv/*.cpp)
file(GLOB_RECURSE SV_HEADERS sv/*.h)


# ------------------------------------------------
# We can finally build and link everything
# ------------------------------------------------
add_executable(vhdlstuff vhdlstuff.cpp ${THINGS_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/version.cpp ${SV_SOURCES})

# the order in which we include libraries matter. libvhdl and liblsp use
# definitions from libcommon, so it should be completely at the back
target_link_libraries(vhdlstuff yaml-cpp::yaml-cpp libvhdl liblsp libcommon libfmt slang::slang args)
target_include_directories(vhdlstuff PRIVATE ${rapidjson_source_dir}/include)

add_executable(debugstuff debug.cpp ${CMAKE_CURRENT_BINARY_DIR}/version.cpp ${SV_SOURCES})

target_link_libraries(debugstuff yaml-cpp::yaml-cpp libvhdl liblsp libcommon libfmt slang::slang args)
target_include_directories(debugstuff PRIVATE ${rapidjson_source_dir}/include)

if(WIN32)
    #do nothing
endif()

if(APPLE)
    #do nothing
endif()

if(MSVC)
    #do nothing
endif(MSVC)

if(LINUX)
    target_link_libraries(vhdlstuff -lpthread -ldl -lstdc++fs)
    target_link_libraries(debugstuff -lpthread -ldl -lstdc++fs)
endif()
