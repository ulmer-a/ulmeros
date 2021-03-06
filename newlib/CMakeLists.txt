cmake_minimum_required(VERSION 3.14)

option(U_NEWLIB "Build Newlib C standard library" OFF)
if(U_NEWLIB)
	add_custom_target(newlib ALL
		${CMAKE_CURRENT_SOURCE_DIR}/build ${ARCH}
		WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
	)
endif()
