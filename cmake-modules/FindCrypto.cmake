# Find the Crypto includes and library
#
#  set CRYPTO_FIND_REQUIRED to require the crypto library
#  otherwise, the user will be given a WITH_CRYPTO option.
# 
#  CRYPTO_INCLUDE_DIR - where to find openssl/sha.h
#  CRYPTO_LIBRARIES   - List of libraries when using crypto
#  CRYPTO_ENABLED     - True if crypto is enabled

INCLUDE(LintelFindLibrary)

LINTEL_FIND_LIBRARY_CMAKE_INCLUDE_FILE(CRYPTO openssl/sha.h crypto)

