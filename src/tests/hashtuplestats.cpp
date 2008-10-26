#include <Lintel/HashTupleStats.hpp>

using namespace std;

typedef boost::tuple<int32_t, bool, string> IBSTuple;

int main() {
#if 0
    IBSTuple tmp(1,true,"a");

    return lintel::tuples::hash(tmp);

    lintel::HashTupleStats<IBSTuple> hts;

    for(int32_t i = 0; i < 10; ++i) {
	hts.add(IBSTuple(i,(i % 2) == 0, str(boost::format("%d") % (i * 10))), 1);
    }
#endif
    
    return 0;
}
