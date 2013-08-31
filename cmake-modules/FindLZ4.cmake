# Find the lz4 and lz4hc includes and library
#
#  set LZ4_FIND_REQUIRED to require the LZ4 library
#  otherwise, the user will be given a WITH_LZ4 option.
#
#  LZ4_INCLUDE_DIR - where to find header file for Variable
#  LZ4_INCLUDES    - all includes needed for Variable
#  LZ4_LIBRARY     - where to find the library for Variable
#  LZ4_LIBRARIES   - all libraries needed for Variable
#  LZ4_ENABLED     - true if VARIABLE is enabled


INCLUDE(LintelFind)

LINTEL_FIND_LIBRARY_CMAKE_INCLUDE_FILE(LZ4 lz4.h lz4)
LINTEL_FIND_LIBRARY_CMAKE_INCLUDE_FILE(LZ4 lz4hc.h lz4hc)
