cmake_minimum_required (VERSION 2.8)
project (nim)

set(nim_VERSION_MAJOR 0)
set(nim_VERSION_MINOR 1)
set(nim_VERSION_PATCH 0)

macro(nim_set_option var default type docstring)
    if(NOT DEFINED ${var})
        set(${var} ${default})
    endif()
    set(${var} ${${var}} CACHE ${type} ${docstring} FORCE)
endmacro()

# nim_set_option(GFS_PROVIDE_FS_ARCHIVER TRUE BOOL "TRUE to have a built-in archiver for filesystem streaming, FALSE to have null archiver")
# nim_set_option(GFS_HAVE_MUTEX FALSE BOOL "TRUE to synchronize member calls, FALSE to explicitly handle synchronization")
# nim_set_option(BUILD_SHARED_LIBS FALSE BOOL "TRUE to build gfs as shared, FALSE to build it as static")

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
elseif(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wpedantic -Werror")
    if (MINGW AND CMAKE_BUILD_TYPE STREQUAL "Release")
        add_definitions(-D__NO_INLINE__)
    endif()
endif()

include_directories(
    nim/include
)

file(GLOB nim_SOURCES
	"nim/src/*.c"
	"nim/src/*.cc"
	"nim/src/*.cpp"
)

add_executable(nim ${nim_SOURCES})