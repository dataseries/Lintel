SET(CMAKE_MODULE_FILES
	FindCrypto.cmake
	FindLibXmlpp.cmake
	FindLintel.cmake
	FindLZO.cmake
        FindLZ4.cmake
        FindSnappy.cmake
	LintelCMakeUtil.cmake
	LintelDocs.cmake
	LintelFind.cmake
)

INSTALL(FILES ${CMAKE_MODULE_FILES}
	DESTINATION ${CMAKE_INSTALL_PREFIX}/share/cmake-modules)
