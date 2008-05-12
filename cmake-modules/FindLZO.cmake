# Find the lzo includes and library
#
#  set LZO_FIND_REQUIRED to require the LZO library
#  otherwise, the user will be given a WITH_LZO option.
# 
#  LZO_INCLUDE_DIR - where to find openssl/sha.h
#  LZO_LIBRARIES   - List of libraries when using LZO
#  LZO_ENABLED     - True if LZO is enabled

INCLUDE(LintelFindLibrary)

# TODO: figure out how to extract the library version; experiments with 
# DataSeries showed that lzo2 could have worse performance than lzo1

LINTEL_FIND_LIBRARY_CMAKE_INCLUDE_FILE(LZO lzo1x.h lzo)
