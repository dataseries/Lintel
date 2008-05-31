SET(CMAKE_MODULE_FILES
	FindCrypto.cmake
	FindLibXmlpp.cmake
	FindLintel.cmake
	FindLZO.cmake
	LintelCMakeUtil.cmake
	LintelFind.cmake
)

INSTALL(FILES ${CMAKE_MODULE_FILES}
	DESTINATION ${CMAKE_INSTALL_PREFIX}/share/cmake-modules)
