#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include <boost/format.hpp>

#include <Lintel/AssertBoost.hpp>
#include <Lintel/StatM.hpp>

using boost::format;
using namespace lintel;

StatM::StatM(pid_t p) : pid(p) {
    if (pid==0) {
	pid = getpid();
    }
    file_name = (format("/proc/%d/statm") % pid).str();
    CHECKED((fp = fopen(file_name.c_str(), "r"))!=0, format("Can't open %s: %s")
	    % file_name % strerror(errno));
    CHECKED(setvbuf(fp, 0, _IONBF, 0)==0, format("Can't set %s unbuffered: %s") 
	    % file_name % strerror(errno));
    updateStatM();
}

StatM::~StatM() {
    SINVARIANT(fclose(fp)==0);
}

void StatM::updateStatM() {
    CHECKED(fseek(fp, 0, SEEK_SET)==0, format("Can't seek %s: %s") % file_name % strerror(errno));
    // format is size resident share text lib data dt
    // lib and dt are not filled in in modern kernels; data is data + stack
    CHECKED(fscanf(fp, "%llu %llu %llu %llu %llu %llu %llu", &res.size, &res.resident,
		   &res.share, &res.text, &res.lib, &res.data, &res.dt) == 7,
	    format("fscanf on %s: %s") % file_name % strerror(errno));
}
