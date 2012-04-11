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
#include <Lintel/TestUtil.hpp>

#define LINTEL_UNSTABLE_PROCESS_STATISTICS_NOWARN
#include <Lintel/unstable/ProcessStatistics.hpp>

using namespace lintel;
using namespace lintel::process_statistics;

using boost::format;

int main() {
    DeptoolInfo deptool_info = getDeptoolInfo();

    const ssize_t page_size = 4096;
    ProcessStatistics proc_stats;
    size_t initial_resident = proc_stats.get(ResidentSize);
    size_t once_resident = ProcessStatistics::getOnce(ResidentSize);
    SINVARIANT(abs(initial_resident - once_resident) < 100*page_size); 
    size_t once_resident_pid = ProcessStatistics::getOnce(ResidentSize, getpid());
    INVARIANT(abs(initial_resident - once_resident_pid) < 100*page_size,
              format("%d %d %.0f\n") % initial_resident % once_resident_pid
              % abs(initial_resident - once_resident_pid));

    size_t initial_size = proc_stats.getCached(AddressSize);
    SINVARIANT(abs(initial_size - ProcessStatistics::getOnce(AddressSize)) < 100*page_size);
    SINVARIANT(abs(initial_size - ProcessStatistics::getOnce(AddressSize, getpid())) 
               < 100*page_size);

    LintelLog::info(format("initially %d/%d") % initial_size % initial_resident);

    char * trash = static_cast<char *>(malloc(page_size * 10000));
    
    SINVARIANT(proc_stats.getCached(AddressSize) == initial_size);  // Check the caching works

    proc_stats.invalidate();
    LintelLog::info(format("after malloc %d/%d") 
		    % proc_stats.getCached(AddressSize) % proc_stats.getCached(ResidentSize));

    if (deptool_info.osVersion() == "opensuse-12.1") {
        LintelLog::warn("ignoring check on resident size, opensuse-12.1 fails it");
    } else {
        // Resident shouldn't change until we touch it.    
        INVARIANT(abs(proc_stats.getCached(ResidentSize) - initial_resident) < 100*page_size,
                  format("failed on %s") % deptool_info.osVersion()); 
    }
    
    // But size should have changed    
    SINVARIANT(abs(proc_stats.getCached(AddressSize) - initial_size) > 9000*page_size);
    memset(trash, 1, page_size * 10000);
    proc_stats.invalidate();
    LintelLog::info(format("after memset %d/%d") 
		    % proc_stats.getCached(AddressSize) % proc_stats.getCached(ResidentSize));
    
    // And now resident should change
    SINVARIANT(abs(proc_stats.get(ResidentSize) - initial_resident) > 9000 * page_size);     
}
