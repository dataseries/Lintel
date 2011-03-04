/*
  Tests the StatM class, which reads /proc/pid/statm and extracts information about memory use.
  It is a bit linux-specific, but if you're not on linux the you can just use getrusage, which
  works on other systems.

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
#include <Lintel/StatM.hpp>

using namespace lintel;

using boost::format;

int main() {
    StatM statm;
    int64_t initial_resident = statm.getStatM().resident;
    int64_t initial_size = statm.getCached().size;
    LintelLog::info(format("initially %d/%d") % initial_size % initial_resident);

    char * trash = static_cast<char *>(malloc(4096 * 10000));
    
    SINVARIANT(statm.getCached().size == initial_size);  // Check the caching works

    statm.getStatM();
    LintelLog::info(format("after malloc %d/%d") 
		    % statm.getCached().size % statm.getCached().resident);

    // Resident shouldn't change until we touch it.    
    SINVARIANT(fabs(statm.getCached().resident - initial_resident) < 100); 
    
    // But size should have changed    
    SINVARIANT(fabs(statm.getCached().size - initial_size) > 9000);
    memset(trash, 1, 4096 * 10000);
    statm.getStatM();
    LintelLog::info(format("after memset %d/%d") 
		    % statm.getCached().size % statm.getCached().resident);
    
    // And now resident should change
    SINVARIANT(fabs(statm.getStatM().resident - initial_resident) > 9000);     
}
