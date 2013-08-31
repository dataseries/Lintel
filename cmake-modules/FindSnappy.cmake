# Find the Snappy includes and library
#
#  set SNAPPY_FIND_REQUIRED to require the SNAPPY library
#  otherwise, the user will be given a WITH_SNAPPY option.
# 
#  SNAPPY_INCLUDE_DIR - where to find header file for Variable
#  SNAPPY_INCLUDES    - all includes needed for Variable
#  SNAPPY_LIBRARY     - where to find the library for Variable
#  SNAPPY_LIBRARIES   - all libraries needed for Variable
#  SNAPPY_ENABLED     - true if VARIABLE is enabled


# TODO: Check if the library was found, installed and configured correctly.
# If not, build and install it first.

INCLUDE(LintelFind)

LINTEL_FIND_LIBRARY_CMAKE_INCLUDE_FILE(SNAPPY snappy-c.h snappy)
