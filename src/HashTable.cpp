/* -*-C++-*-
   (c) Copyright 2002-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    prime list
*/

#include <Lintel/HashTable.hpp>

// Set to be about 4x increment each time; resizing the hash table is 
// expensive, and we only pay 4 bytes/entry
uint32_t HashTable_prime_list[] = {
  5, 23, 107, 
  433, 1543, 6091, 24281, 100169, 487651, 1179589, 2471093, 
  7368787, 32452843, 141650939, 0
};

