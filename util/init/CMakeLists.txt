cmake_minimum_required(VERSION 3.14)

if(U_INIT)
    message(STATUS "building /bin/init")
    add_custom_target(init
	    mkdir -p ${SYSROOT}/bin
	    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/init ${SYSROOT}/bin/init
    )
    add_dependencies(userspace init)
endif()
