#include <Lintel/Tuples.hpp>

using namespace std;
using boost::format;
using lintel::hash;

typedef boost::tuple<bool, char, int32_t, int64_t, string> Tuple;
typedef lintel::tuples::TupleToAnyTuple<Tuple>::type AnyTuple;
typedef lintel::tuples::BitsetAnyTuple<Tuple> BitsetanyTuple;

int main() {
    Tuple tuple1(true, 'a', 7, 123456789023LL, "Hello, World");
    // alternate so attributes are <, >, <, >, <
    Tuple tuple2(false, 'b', 6, 123456789022LL, "Iello, World");

    INVARIANT(hash(tuple1) == 2462433311U, format("got %d?") % hash(tuple1));
    INVARIANT(hash(tuple2) == 1460156332U, format("got %d?") % hash(tuple2));

    SINVARIANT(tuple2 < tuple1);

    AnyTuple anytuple1; 
    assign(anytuple1, tuple1);
    AnyTuple anytuple2;
    assign(anytuple2, tuple2);

    INVARIANT(hash(anytuple1) == 2462433311U, format("%d?") % hash(anytuple1));
    INVARIANT(hash(anytuple2) == 1460156332U, format("%d?") % hash(anytuple2));

    SINVARIANT(anytuple2 < anytuple1);
    anytuple2.get<0>().any = true;
    SINVARIANT(anytuple1 < anytuple2);
    anytuple1.get<0>().any = true;
    SINVARIANT(anytuple1 < anytuple2);
    INVARIANT(hash(anytuple1) == 2138559752U, format("%d?") % hash(anytuple1));
    INVARIANT(hash(anytuple2) == 1910363323U, format("%d?") % hash(anytuple2));

    anytuple1.get<1>().any = true;
    anytuple2.get<1>().any = true;
    SINVARIANT(anytuple2 < anytuple1);

    INVARIANT(hash(anytuple1) == 1248248738U, format("%d?") % hash(anytuple1));
    INVARIANT(hash(anytuple2) == 1959181482U, format("%d?") % hash(anytuple2));

    BitsetanyTuple bitsetanytuple1(tuple1);
    BitsetanyTuple bitsetanytuple2(tuple2);

    SINVARIANT(bitsetanytuple2 < bitsetanytuple1);
    bitsetanytuple2.any[0] = true;
    SINVARIANT(bitsetanytuple1 < bitsetanytuple2);
    bitsetanytuple1.any[0] = true;
    SINVARIANT(bitsetanytuple1 < bitsetanytuple2);
    INVARIANT(hash(bitsetanytuple1) == 2138559752U, 
	      format("%d?") % hash(bitsetanytuple1));
    INVARIANT(hash(bitsetanytuple2) == 1910363323U, 
	      format("%d?") % hash(bitsetanytuple2));

    bitsetanytuple1.any[1] = true;
    bitsetanytuple2.any[1] = true;
    SINVARIANT(bitsetanytuple2 < bitsetanytuple1);

    INVARIANT(hash(bitsetanytuple1) == 1248248738U, format("%d?") 
	      % hash(bitsetanytuple1));
    INVARIANT(hash(bitsetanytuple2) == 1959181482U, format("%d?") 
	      % hash(bitsetanytuple2));

    return 0;
}
