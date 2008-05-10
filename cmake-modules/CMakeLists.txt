SET(CMAKE_MODULE_FILES
	FindLibXmlpp.cmake
	FindLintel.cmake
	LintelCMakeUtil.cmake
)

INSTALL(FILES ${CMAKE_MODULE_FILES}
	DESTINATION ${CMAKE_INSTALL_PREFIX}/share/cmake-modules)
