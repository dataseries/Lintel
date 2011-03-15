#ifndef LINTEL_UNSTABLE_PROCESS_STATISTICS_HPP
#define LINTEL_UNSTABLE_PROCESS_STATISTICS_HPP

#warning Including unstable header ProcessStatistics.hpp; liable to be deprecated without warning

#include <sys/types.h>
#include <string>

namespace lintel {
    enum StatType {
	Invalid = 0,
	AddressSize,  /// Size of the address space, in bytes
	ResidentSize  /// Size of the resident set (pages physically allocated) in bytes
    };
    
    /// Returns statistics about a process, of the sort you can get from /proc/pid on linux, or
    /// getrusage on some other unixes.  Note that the meaning of some fields may vary slightly from
    /// platform to platform; e.g. how to count shared pages in resident set size can be subtly
    /// different between different OSes
    class ProcessStatistics {
    public:
	    	
	/// 0 == just use getpid().
	ProcessStatistics(pid_t p=0);

	~ProcessStatistics();


	/// Get the value of StatType which; if it is expensive to do be fresh then it is acceptible
	/// to return a stale result.
	const size_t getCached(StatType which);

	/// Get the value of StatType which; make sure the result is fresh.
	size_t get(StatType which);

	/// Invalidate all cached statistics.
	void invalidate();

	/// Is this StatType supported on this platform
	bool supported(StatType which);

	/// Get the value of StatType which as a one-off.
	static size_t getOnce(StatType which, pid_t p = 0) {
	    ProcessStatistics temp(p);
	    return temp.get(which);
	}

    private:  // TODO: make this hide implementatin specific details better; e.g. through an impl
	      // class

	// A lot below here is linux-specific	
	typedef struct {
	    size_t size;
	    size_t resident;
	    size_t share;
	    size_t text;
	    size_t lib; // Not filled in in modern kernels
	    size_t data;
	    size_t dt;  // Not filled in in modern kernels
	} ProcessStatisticsRes;


	pid_t pid;
	std::string file_name;
	FILE * fp;	
	ProcessStatisticsRes res;
    };
}
   
#endif
