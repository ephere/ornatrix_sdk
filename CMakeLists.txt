cmake_minimum_required( VERSION 3.13 )

project( Ephere.Ornatrix )

set( CMAKE_CXX_STANDARD 11 )

# Remove the default MinSizeRel and RelWithDebInfo configurations provided by CMake
set( CMAKE_CONFIGURATION_TYPES Debug;Release CACHE STRING "Configuration types" FORCE )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/Debug" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/Debug" )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/Release" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/Release" )

if( WIN32 )
	add_definitions( -D_SECURE_SCL_DEPRECATE=0 -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS -D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING )
endif( WIN32 )

add_library( Ephere.Ornatrix INTERFACE IMPORTED )
set_target_properties( Ephere.Ornatrix PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( Ephere.Ornatrix.Test Main.cpp )

target_link_libraries( Ephere.Ornatrix.Test PRIVATE Ephere.Ornatrix )
if( NOT WIN32 AND NOT APPLE )
	target_link_libraries( Ephere.Ornatrix.Test PRIVATE dl )
endif()

add_custom_command( TARGET Ephere.Ornatrix.Test POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Ephere.Ornatrix.Test> ${CMAKE_CURRENT_SOURCE_DIR}/bin )
