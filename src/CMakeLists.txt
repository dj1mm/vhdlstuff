find_package(Python3 REQUIRED)


# Helper function to extract the directory path of each files. Shamefully taken
# from https://raw.githubusercontent.com/verilog-to-routing/vtr-verilog-to-routing/master/cmake/modules/FilesToDirs.cmake

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

function(files_to_dirs file_list directory_list)
    # Extract the directories for each header file
    foreach(file ${${file_list}})
        get_filename_component(dir ${file} DIRECTORY)
        list(APPEND dir_list ${dir})
    endforeach()

    # Remove any duplicates
    list(LENGTH "${dir_list}" length)
    if(${length} GREATER 1) # Avoid error with zero-length lists
        list(REMOVE_DUPLICATES ${dir_list})
    endif()

    # Set the second argument in the caller's scope
    set(${directory_list} ${dir_list} PARENT_SCOPE)
endfunction(files_to_dirs)


include(ExternalProject)

# Download and install args
ExternalProject_Add(
    args
    URL https://github.com/Taywee/args/archive/master.zip
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/args
    INSTALL_COMMAND "" # Disable install step
)

# Get args source and binary directories from CMake project
ExternalProject_Get_Property(args source_dir binary_dir)
set(args_source_dir ${source_dir})
set(args_binary_dir ${binary_dir})

# ------------

include(ExternalProject)

# Download and install yaml-cpp
# https://stackoverflow.com/questions/6351609/
# https://cmake.org/pipermail/cmake/2015-September/061548.html
ExternalProject_Add(
    yamlcpp_project
    URL https://github.com/jbeder/yaml-cpp/archive/master.zip
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/yamlcpp_project
    CMAKE_ARGS "-DYAML_CPP_BUILD_TESTS=OFF"
    INSTALL_COMMAND "" # Disable install step
)

# Get yamlcpp source and binary directories from CMake project
ExternalProject_Get_Property(yamlcpp_project source_dir binary_dir)
set(yamlcpp_source_dir ${source_dir})
set(yamlcpp_binary_dir ${binary_dir})

add_library(yamlcpp STATIC IMPORTED)
SET_TARGET_PROPERTIES(yamlcpp PROPERTIES PREFIX "")
set_target_properties(yamlcpp PROPERTIES IMPORTED_LOCATION ${yamlcpp_binary_dir}/${CMAKE_STATIC_LIBRARY_PREFIX}yaml-cpp${CMAKE_STATIC_LIBRARY_SUFFIX})

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

files_to_dirs(THINGS_HEADERS INCLUDE_DIRS)

# ------------------------------------------------
# We can finally build and link everything
# ------------------------------------------------
add_executable(vhdlstuff vhdlstuff.cpp ${THINGS_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/version.cpp)

# the order in which we include libraries matter. libvhdl and liblsp use
# definitions from libcommon, so it should be completely at the back
target_link_libraries(vhdlstuff yamlcpp libvhdl liblsp libcommon libfmt slang::slang)
target_include_directories(vhdlstuff PRIVATE ${args_source_dir}/ ${yamlcpp_source_dir}/include ${yamlcpp_source_dir}-build/include ${rapidjson_source_dir}/include)

add_executable(debugstuff debug.cpp ${CMAKE_CURRENT_BINARY_DIR}/version.cpp)

target_link_libraries(debugstuff yamlcpp libvhdl liblsp libcommon libfmt slang::slang)
target_include_directories(debugstuff PRIVATE ${args_source_dir}/ ${yamlcpp_source_dir}/include ${yamlcpp_source_dir}-build/include ${rapidjson_source_dir}/include)

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
