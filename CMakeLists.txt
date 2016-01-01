cmake_minimum_required(VERSION 2.8)
set( CMAKE_CONFIGURATION_TYPES "Debug;Release" )
project(synacor_challenge)

if( NOT CMAKE_BUILD_TYPE )
	set( CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: ${CMAKE_CONFIGURATION_TYPES}" FORCE )
endif( )

MESSAGE ( "Building for ${CMAKE_BUILD_TYPE}" )
find_package(Boost 1.59.0 REQUIRED COMPONENTS iostreams system thread date_time filesystem )

set( COMPILER_SPECIFIC_LIBS )
MESSAGE( WARNING "Current Compiler:" ${CMAKE_CXX_COMPILER_ID} )
IF( ${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC" )
	MESSAGE( WARNING "Using Visual Studio" )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0601 /MP" )
	set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG" )
	set(Boost_LIBRARIES )
ELSEIF( ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" )
	MESSAGE( WARNING "Using Clang" )
	find_package(Threads REQUIRED)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -stdlib=libc++ -g -O3 -Weverything -Wno-c++98-compat  -Wfatal-errors -Wno-covered-switch-default -Wno-padded -Wno-exit-time-destructors -Wno-c++98-compat-pedantic -Wno-unused-parameter -Wno-missing-noreturn -Wno-missing-prototypes -Wno-disabled-macro-expansion")
	set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g -O1" )
	set(CMAKE_CXX_FLAGS_RELEASE "-O3" )
ELSEIF( ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" )
	MESSAGE( WARNING "Using G++" )
	find_package(Threads REQUIRED)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wno-deprecated-declarations")
	set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g -O1" )
	set(CMAKE_CXX_FLAGS_RELEASE "-O3" )
ENDIF( )

if(THREADS_HAVE_PTHREAD_ARG)
	set(COMPILER_SPECIFIC_LIBS, "-lpthread" )
endif( )

set( SOURCE_FILES
	console.cpp
	console.h
	file_helper.cpp
	file_helper.h
	helpers.h
	memory_helper.h
	vm.cpp
	vm.h
	vm_control.cpp
	vm_control.h
)

include_directories( SYSTEM ${Boost_INCLUDE_DIRS} )
link_directories( ${Boost_LIBRARY_DIRS} )

add_executable(synacor_challenge ${SOURCE_FILES} main.cpp )
target_link_libraries(synacor_challenge ${Boost_LIBRARIES} ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${COMPILER_SPECIFIC_LIBS} )

add_executable( to_assembler ${SOURCE_FILES} to_assembler.cpp )
target_link_libraries( to_assembler ${Boost_LIBRARIES} ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${COMPILER_SPECIFIC_LIBS} )

