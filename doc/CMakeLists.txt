CMAKE_MINIMUM_REQUIRED(VERSION 2.4.4 FATAL_ERROR)
INCLUDE(FindDoxygen)

# For windows documentation install, see:
# http://www.cmake.org/pipermail/cmake/2006-August/010786.html

CONFIGURE_FILE(doxygen.config.in ${CMAKE_CURRENT_BINARY_DIR}/doxygen.config @ONLY)

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/doxygen/html/index.html
	           COMMAND ${DOXYGEN_EXECUTABLE} 
			   ${CMAKE_CURRENT_BINARY_DIR}/doxygen.config
		   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/doxygen.config)

ADD_CUSTOM_TARGET(doc ALL 
		  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/doxygen/html/index.html)

# TODO: figure out how to clean; the following doesn't work.
# SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES doxygen)

# INSTALL is not scriptable, so we can't have an INSTALL(DIRECTORY ...)
# bit either in a code block or a script block.  We also can't have just
# an install target because the files don't exist until after we do the
# build.

#INSTALL(CODE "INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doxygen/html DESTINATION ${CMAKE_INSTALL_PREFIX}/share/doc/Lintel/)")

INSTALL(CODE "EXEC_PROGRAM(${CMAKE_CURRENT_SOURCE_DIR}/install-doxygen.sh ARGS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_INSTALL_PREFIX}/share)")
 