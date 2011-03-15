/*
  Tests the ProcessStatistics class, which gets statistics about a process of the sort
  available in /proc/pid on linux, or through getrusage.  Currently only implemented
  for linux

  1) we get our initial memory use

  2) we make an allocation and then check the cached address space size didn't change

  3) we check that the refreshed address space size changes, but the resident size doesn't.  This is
     dependent on linux (& most other modern systems) not actually allocating pages until you write
     to them

  4) we write to the memory, and check that the resident size has now changed.
 */
#include <math.h>

#include <boost/format.hpp>

#include <Lintel/AssertBoost.hpp>
#include <Lintel/LintelLog.hpp>
#include <Lintel/unstable/ProcessStatistics.hpp>

using namespace lintel;

using boost::format;

int main() {
    const size_t page_size = 4096;
    ProcessStatistics proc_stats;
    size_t initial_resident = proc_stats.get(ResidentSize);
    size_t initial_size = proc_stats.getCached(AddressSize);
    LintelLog::info(format("initially %d/%d") % initial_size % initial_resident);

    char * trash = static_cast<char *>(malloc(page_size * 10000));
    
    SINVARIANT(proc_stats.getCached(AddressSize) == initial_size);  // Check the caching works

    proc_stats.invalidate();
    LintelLog::info(format("after malloc %d/%d") 
		    % proc_stats.getCached(AddressSize) % proc_stats.getCached(ResidentSize));

    // Resident shouldn't change until we touch it.    
    SINVARIANT(fabs(proc_stats.getCached(ResidentSize) - initial_resident) < 100*page_size); 
    
    // But size should have changed    
    SINVARIANT(fabs(proc_stats.getCached(AddressSize) - initial_size) > 9000*page_size);
    memset(trash, 1, page_size * 10000);
    proc_stats.invalidate();
    LintelLog::info(format("after memset %d/%d") 
		    % proc_stats.getCached(AddressSize) % proc_stats.getCached(ResidentSize));
    
    // And now resident should change
    SINVARIANT(fabs(proc_stats.get(ResidentSize) - initial_resident) > 9000 * page_size);     
}
