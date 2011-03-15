#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include <boost/format.hpp>

#include <Lintel/AssertBoost.hpp>
#include <Lintel/unstable/ProcessStatistics.hpp>

using boost::format;
using namespace lintel;

ProcessStatistics::ProcessStatistics(pid_t p) : pid(p) {
    if (pid==0) {
	pid = getpid();
    }
    file_name = (format("/proc/%d/statm") % pid).str();
    CHECKED((fp = fopen(file_name.c_str(), "r"))!=0, format("Can't open %s: %s")
	    % file_name % strerror(errno));
    CHECKED(setvbuf(fp, 0, _IONBF, 0)==0, format("Can't set %s unbuffered: %s") 
	    % file_name % strerror(errno));
    invalidate();
}

ProcessStatistics::~ProcessStatistics() {
    SINVARIANT(fclose(fp)==0);
}

const size_t ProcessStatistics::getCached(StatType which) {
    const size_t page_size = 4096; // Assumes 4K pages size... is there a good way to get page size?
    switch (which) {
    case AddressSize:
	return res.size * page_size;
    case ResidentSize:
	return res.resident * page_size;
    default:
	FATAL_ERROR(format("%d is an invalid type of statistic to ask for") % which);
    }
}

size_t ProcessStatistics::get(StatType which) {
    invalidate();
    return getCached(which);
}

bool ProcessStatistics::supported(StatType which) {
    switch (which) {
    case AddressSize:
    case ResidentSize:
	return true;
    default:
	return false;
    }
}

void ProcessStatistics::invalidate() {
    CHECKED(fseek(fp, 0, SEEK_SET)==0, format("Can't seek %s: %s") % file_name % strerror(errno));
    // format is size resident share text lib data dt
    // lib and dt are not filled in in modern kernels; data is data + stack
    CHECKED(fscanf(fp, "%zu %zu %zu %zu %zu %zu %zu", &res.size, &res.resident,
		   &res.share, &res.text, &res.lib, &res.data, &res.dt) == 7,
	    format("fscanf on %s: %s") % file_name % strerror(errno));
}
