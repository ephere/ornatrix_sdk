add_executable( Ephere.Ornatrix.Test Main.cpp )

target_link_libraries( Ephere.Ornatrix.Test PRIVATE Ephere.Ornatrix )
if( NOT WIN32 AND NOT APPLE )
	target_link_libraries( Ephere.Ornatrix.Test PRIVATE dl )
endif()

add_custom_command( TARGET Ephere.Ornatrix.Test POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Ephere.Ornatrix.Test> ${CMAKE_CURRENT_SOURCE_DIR}/../bin )
