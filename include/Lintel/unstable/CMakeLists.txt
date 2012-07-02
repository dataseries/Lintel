SET(UNSTABLE_INCLUDES
        InterpolationSearch.hpp
	ProcessStatistics.hpp
)

INSTALL(FILES ${UNSTABLE_INCLUDES}
	DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Lintel/unstable)

