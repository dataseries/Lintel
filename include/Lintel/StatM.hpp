#ifndef LINTEL_STAT_M_HPP
#define LINTEL_STAT_M_HPP

#include <sys/types.h>
#include <string>

// TODO-joe: We may need to chat, but I think we would be better off making this class a general
// interface that will get resource usage and will do it the right way on various platforms, i.e.
// make this class ResourceUsage, and probably use the private implementation pattern to do
// the actual implementations.  We can of course only do the linux one for now, but this approach
// will let us accept patches for other OSs, e.g freebsd, solaris.

namespace lintel {

    /// Encapsulates reading things from /proc/pid/statm
    class StatM {
    public:
	/// Parsed statm file; all numbers are in pages
	typedef struct {
            // TODO-joe: can we use size_t for all of these?
	    long long unsigned int size;
	    long long unsigned int resident;
	    long long unsigned int share;
	    long long unsigned int text;
	    long long unsigned int lib; // Not filled in in modern kernels
	    long long unsigned int data;
	    long long unsigned int dt;  // Not filled in in modern kernels
	} StatMRes;
	
	/// 0 == just use getpid().
	StatM(pid_t p=0);

	~StatM();

	const StatMRes getStatM() {
	    updateStatM();
	    return res;
	}
	
	const StatMRes getCached() const {
	    return res;
	}

	static StatMRes getOnce(pid_t p = 0) {
	    StatM temp(p);
	    return temp.getCached();
	}

    private:
	void updateStatM();

	pid_t pid;
	std::string file_name;
	// FILE *, because streams error handling stinks like week old fish.  It isn't even
	// uniformly C++ like, because to get *why* you failed you must use C's errno and strerror
	// And even with this comment this is one line shorter than the streams interface would be
	FILE * fp;	
	StatMRes res;
    };
}
   
#endif
