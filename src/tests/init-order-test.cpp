#include <stdio.h>
#include <vector>

// Fails on all C++ implementations I can easily try:
// Debian gcc 2.95-4.1; HP-UX aCC A.03.52

using namespace std;
void initializer_order_test();

class fiz {
public:
    fiz() {
	initializer_order_test();
    }
};

class fuz {
public:
    fuz() {
    }
    void init(unsigned i) {
	tmp.push_back(i);
	++tmp2;
    }
    vector<int> tmp;
    unsigned tmp2;
};

fiz a, b, c;
fuz d;
fiz e, f, g;

static unsigned n_iot;

void
initializer_order_test()
{
    d.init(n_iot);
    ++n_iot;
}

int
main()
{
    printf("Counts: calls to iot=%d, count in class=%d, size of vector=%d\n",
	   n_iot, d.tmp2, d.tmp.size());
    if (n_iot == d.tmp.size()) {
	printf("Safe to setup hash tables statically and call add %d == %d\n",
	       n_iot, d.tmp.size());
    } else {
	printf("Not safe to setup hash tables statically and call add %d != %d\n",
	       n_iot, d.tmp.size());
    }

}
