cmake_minimum_required(VERSION 3.14)

if(U_NEWLIB)
	add_custom_target(newlib
		${CMAKE_CURRENT_SOURCE_DIR}/build ${ARCH}
		WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
	)
	add_dependencies(userspace newlib)
endif()
