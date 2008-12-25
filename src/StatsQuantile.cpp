/* -*-C++-*-
   (c) Copyright 2005-2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    StatsQuantile implementation
*/

#include <algorithm>
#include <sstream>
#include <cstring>

#include <boost/format.hpp>

#include <Lintel/AssertBoost.hpp>
#include <Lintel/Double.hpp>
#include <Lintel/PriorityQueue.hpp>
#include <Lintel/StatsQuantile.hpp>

using namespace std;

namespace {
    double fact(double a) {
	double ret = 1.0;
	for(double i = 2;i<=a;i++) {
	    ret *= i;
	}
	return ret;
    }

    double choose(int b, int a) { // b choose a 
	double ret = fact(b) / (fact(b-a) * fact(a));
	return ret;
    }

    class doubleOrder {
    public:
	inline bool operator() (double a, double b) const {
	    return a < b;
	}
    };
    
}

StatsQuantile::StatsQuantile(double _quantile_error, long long in_nbound, int _print_nrange)
    : quantile_error(_quantile_error), Nbound(in_nbound), print_nrange(_print_nrange)
{
    // sanity check that we haven't been called in a weird/wrong way.
    INVARIANT(quantile_error < 0.2, 
	      boost::format("whoa, quantile_error %.3g >= 0.2??") 
	      % quantile_error);
    double tmp_nbound = Nbound; // easy for calculations
    INVARIANT(tmp_nbound >= 100, boost::format("whoa, nbound of %.0f?") % tmp_nbound);
    // The hunt for b = nbuffers, and k = buffer_size is on, following 
    // section 4.5 in the paper

    int best_b = -1, best_k = -1;
    const int max_b = 30;
    const int max_h = 40;
    for(int b=2;b<max_b;b++) {
	int h = 3;
	for(;h < max_h;h++) {
	    double v = (h-2) * choose(b+h-2,h-1) - choose(b+h-3,h-3) + choose(b+h-3,h-2);
	    if (v > 2 * quantile_error * tmp_nbound) {
		--h; // this one is too high
		break;
	    }
	}
	double k = ceil(tmp_nbound / choose(b+h-2,h-1));
	if (b * k < 1.0e9) {
	    if (best_b == -1 || b * k < best_b * best_k) {
		best_b = b;
		best_k = (int)k;
	    } 
	}
    }
    
    INVARIANT(best_b > 0 && best_k > 0,
	      boost::format("unable to find b/k for %.8g %.8g")
	      % quantile_error % tmp_nbound);
    // lower bound sanity value
    if (best_k < 10) best_k = 10;
    // First requirement makes the logic setting the buffer_level faster
    // second one just seems sane.
    init(best_b, best_k);
}

// fake Nbound, big enough for tests
StatsQuantile::StatsQuantile(const string &type_disambiguate,
			     int _nbuffers, int _buffer_size, int _print_nrange)
    : quantile_error(Double::NaN), Nbound(5000000), print_nrange(_print_nrange)
{
    init(_nbuffers, _buffer_size);
}

void StatsQuantile::init(int _nbuffers, int _buffer_size) {
    nbuffers = _nbuffers;
    buffer_size = _buffer_size;
    INVARIANT(nbuffers > 1,
	      "need at least two buffers for code to work correctly");
    INVARIANT(buffer_size >= 10,
	      "buffers smaller than 10 don't make sense.");
    all_buffers = new one_buffer[nbuffers];
    for(int i = 0; i < nbuffers; ++i) {
	all_buffers[i] = NULL;
    }
    buffer_weight = new int64_t[nbuffers];
    buffer_level = new int[nbuffers];
    buffer_sorted = new bool[nbuffers];
    tmp_buffer = new double[buffer_size];
    collapse_pos = new int[buffer_size];
    init_buffers();
}

void StatsQuantile::init_buffers() {
    for(int i=0;i<nbuffers;i++) {
	if (all_buffers[i] == NULL) {
	    all_buffers[i] = new double[buffer_size];
	}
	// touch all the space to make sure it is allocated; necessary
	// to avoid demand allocation (which can create long delays
	// when used in Buttress2)
	for(int j=0;j<buffer_size;j+=32) {
	    all_buffers[i][j] = 0.0;
	}
	buffer_weight[i] = -1;
	buffer_level[i] = -1;
	buffer_sorted[i] = false;
    }
    cur_buffer = 0;
    cur_buffer_pos = 0;
    buffer_weight[cur_buffer] = 1;
    buffer_level[cur_buffer] = 0;
    collapse_even_low = true;
}

StatsQuantile::~StatsQuantile() {
    for(int i=0;i<nbuffers;i++) {
	delete[] all_buffers[i];
    }
    delete[] all_buffers;
    delete[] buffer_weight;
    delete[] buffer_level;
    delete[] buffer_sorted;
    delete[] tmp_buffer;
    delete[] collapse_pos;
}

void StatsQuantile::reset() {
    Stats::reset();
    init_buffers();
}

void StatsQuantile::addQuantile(const double value) {
    if (cur_buffer_pos >= buffer_size) {
	cur_buffer += 1;
	cur_buffer_pos = 0;
	if (cur_buffer == nbuffers) {
	    collapse();
	} else {
#if 1
	    // do the sort here to amortize the work
	    sort(all_buffers[cur_buffer - 1],
		      all_buffers[cur_buffer - 1]+buffer_size,
		      doubleOrder());
	    buffer_sorted[cur_buffer-1] = true;
#endif
	}
	buffer_weight[cur_buffer] = 1;
	buffer_level[cur_buffer] = 0;
	if (cur_buffer == nbuffers - 1) {
	    buffer_level[cur_buffer] = buffer_level[cur_buffer - 1];
	}
    }
    buffer_sorted[cur_buffer] = false;
    all_buffers[cur_buffer][cur_buffer_pos] = value;
    cur_buffer_pos += 1;
}

void StatsQuantile::add(const double value) {
    Stats::add(value);
    addQuantile(value);
}

void StatsQuantile::add(const Stats &_stat) {
    const StatsQuantile *stat = dynamic_cast<const StatsQuantile *>(&_stat);
    INVARIANT(stat != NULL, "need another StatsQuantile for add(Stats) to work");

    Stats::add(_stat); 

    if (cur_buffer == 0 && cur_buffer_pos == 0 && // we are empty
	buffer_size == stat->buffer_size && // we are compatible
	nbuffers == stat->nbuffers) {
	INVARIANT(countll() == _stat.countll(), "??");
	INVARIANT(stat->cur_buffer < nbuffers, "??");
	for(int i=0; i <= stat->cur_buffer; ++i) {
	    // Will copy some useless stuff in last buffer.
	    memcpy(all_buffers[i], stat->all_buffers[i],
		   sizeof(double) * buffer_size);
	    buffer_weight[i] = stat->buffer_weight[i];
	    buffer_level[i] = stat->buffer_level[i];
	    buffer_sorted[i] = stat->buffer_sorted[i];
	}
	cur_buffer = stat->cur_buffer;
	cur_buffer_pos = stat->cur_buffer_pos;
	// TODO: determine if we really have to copy collapse_pos
	memcpy(collapse_pos, stat->collapse_pos, sizeof(int) * buffer_size);
	collapse_even_low = stat->collapse_even_low;
	return;
    }

    // There should be a faster way to do this, but this should work
    // correctly.  The faster way should be possible if for no other
    // reason we are inserting the values in order.  In essence we
    // pretent that stat was made up of repeated insertions of the
    // values kept.  Since it's contents are indistinguishable from
    // that actually happening, the merge should be pretty close to
    // correct; this is of course not a formal proof, and I suspect
    // that it is wrong in some special case.

    // TODO: special case bulk loading of lots of the same values;
    // should be able to greatly simplify the addQuantile code to just
    // copy a swath of values in.  Probably can even sort the buffer
    // first, and if we are in order, then just drop everything in and
    // leave the buffer marked sorted.

    for(int i=0; i < stat->cur_buffer; ++i) {
	// cur_buffer is the one we are filling, so all previous ones are full
	double *abuf = stat->all_buffers[i];
	for(int j=0; j < stat->buffer_size; ++j) {
	    double v = abuf[j];
	    for(int64_t k=0; k < stat->buffer_weight[i]; ++k) {
		addQuantile(v);
	    }
	}
    }
    double *abuf = stat->all_buffers[stat->cur_buffer];
    for (int j=0; j < stat->cur_buffer_pos; ++j) {
	double v = abuf[j];
	for(int64_t k=0; k < stat->buffer_weight[stat->cur_buffer]; ++k) {
	    addQuantile(v);
	}
    }
}

// getQuantile is quite expensive for the upper quantiles; in theory
// we ought to be able to do some sort of binary-like search in that case,
// skipping by a lot of the lower quantiles in a batch rather than 
// ratcheting through them one by one as in the paper, and the current
// implementation

#if STATSQUANTILE_TIMING
#define SQTIMING(x) x
#else
#define SQTIMING(x) 
#endif

// const because the only operation on class variables is a sort of buffers
double StatsQuantile::getQuantile(double quantile, bool allow_invalid_nbound) const {
    int64_t count = countll();
    if (count == 0) {
	return Double::NaN;
    }
    INVARIANT(count <= Nbound || allow_invalid_nbound,
	      boost::format("Error: %d (# entries in quantile) > %d (Nbound on quantile)")
	      % count % Nbound);
    SQTIMING(Clock::T gq_time_0 = Clock::now();)
    INVARIANT(quantile >= 0.0 && quantile <= 1.0, "quantile out of bounds");
    // this is slightly different than the algorithm in the paper; the 
    // goal is to allow people to calculate quantiles while the algorithm
    // is running without having to add in and remove the +-\Inf entries
    // that would be needed to make the last buffer have integral size
    // I think this change is correct

    for(int i = 0; i <= cur_buffer; i++) {
	collapse_pos[i] = 0;
	// TODO: add a check that the total weight covered by all the
	// current buffers is exactly the number of elements that we
	// have.  Have to be a little careful on the last buffer to
	// only count the values that we have actually inserted.
	if (!buffer_sorted[i]) {
	    if (i < cur_buffer) {
		sort(all_buffers[i],all_buffers[i] + buffer_size,
			  doubleOrder());
	    } else {
		// only sort part that has values in it.
		sort(all_buffers[i],all_buffers[i] + cur_buffer_pos,
			  doubleOrder());
		// Fill the rest with Inf's so that we don't have to
		// do any special logic when choosing the "smallest"
		// next value, all of the Inf's will come at the end, and
		// if we get to an Inf, then we are done anyway and it
		// doesn't matter if we take one that doesn't actually exist.
		for(int j=cur_buffer_pos;j<buffer_size;j++) {
		    all_buffers[i][j] = Double::Inf;
		}
	    }
	    buffer_sorted[i] = true;
	}
    }
    SQTIMING(Clock::T gq_time_1 = Clock::now(); 
	     // accum_gq_init += gq_time_1 - gq_time_0;
	     )
    double prev_val = -Double::Inf;
    // the 1e-15 accounts for a minor rounding error that seems to occur
    // when doing division, e.g. ceil(150000.0 * (131058.0 / 150000.0)) on
    // linux = 131059.0; I'd imagine we'll never see 1e+15 values
    // wherein this adjustment would make a difference
    double nentries = countll();
    INVARIANT(nentries < 1e+14,
	      "Rounding error adjustment may start to do something wrong around 1e+14 entries.\n"
	      "May be safe to decrease rounding adjustment to 0.5e-15, but will be getting very\n"
	      "close to the limit of precision in floating point.  How did you get this many\n"
	      "entries into the table??");
    // Subtract 1 because the quantile positions count from 1..n, but
    // C++ arrays index from 0..n-1
    int64_t target_index 
	= static_cast<int64_t>(ceil(nentries * (quantile - 1e-15))) - 1;
    if (target_index < 0) target_index = 0;
    if (target_index == nentries) {
	target_index = (long long)(nentries - 1);
    }
    int64_t cur_index = 0;

    double second_min_val = Double::Inf;
    int second_min_buffer = -1;
    SQTIMING(Clock::T gq_time_searchstart = Clock::now();)
    while(1) {
	SQTIMING(Clock::T gq_time_2 = Clock::now();
		 accum_gq_nelem += 1;)
	double min_val = Double::Inf;
	int min_buffer = -1;
	// TODO: re-do this using the replaceTop stuff as done in
	// collapse, it works much faster since it's halved the
	// constants on the selection relative to the time when the
	// previous comment went in.


	// find the buffer with the next smallest entry remaining
	// tried using the Buttress2 priority queue to do this, but it
	// turned out to take about the same time as just doing the
	// scan on a 12x229 set, adding the second min buffer trick
	// got about 25% performance improvement, but adding in the
	// third min val tracking would be obnoxiously complicated, so
	// we're not going to do that.
	if (second_min_buffer >= 0) {
	    min_val = second_min_val;
	    min_buffer = second_min_buffer;
	    second_min_buffer = -1;
	} else {
	    second_min_val = Double::Inf;
	    second_min_buffer = -1;
	    for(int i=0;i<=cur_buffer;i++) {
		if (collapse_pos[i] < buffer_size) {
		    double v = all_buffers[i][collapse_pos[i]];
		    if (v < min_val) {
			second_min_val = min_val;
			second_min_buffer = min_buffer;
			min_val = v;
			min_buffer = i;
		    } else if (v < second_min_val) {
			second_min_val = v;
			second_min_buffer = i;
		    }
		}
	    }
	    int x = collapse_pos[min_buffer] + 1;
	    if (x < buffer_size &&
		all_buffers[min_buffer][x] < second_min_val) {
		second_min_val = all_buffers[min_buffer][x];
		second_min_buffer = min_buffer;
	    }
	    //	    second_min_buffer = -1;
	}
	INVARIANT(min_buffer >= 0 && min_buffer < buffer_size,
		  boost::format("Whoa, no minimal buffer?!;"
				" searching for posn %lldd, cur %lldd")
		  % target_index % cur_index);
	collapse_pos[min_buffer] += 1;
	INVARIANT(min_val >= prev_val, "Whoa, sort order error?!");
	SQTIMING(Clock::T gq_time_3 = Clock::now(); 
		 accum_gq_inner += gq_time_3 - gq_time_2;)
	// this entry occupies sorted order positions
	// [cur_index .. cur_index + buffer_weight[min_buffer] - 1]
	// so the check is for cur_index > target_index
	// need to think about the logic for this in collapse()
	cur_index += buffer_weight[min_buffer];
	if (cur_index > target_index) {
	    SQTIMING(Clock::T gq_time_4 = Clock::now();
		     accum_gq_search += gq_time_4 - gq_time_searchstart;
		     accum_gq_all += gq_time_4 - gq_time_0;)
	    return min_val;
	}
	SQTIMING(accum_gq_init += Clock::now() - gq_time_3;)
    }
    return Double::NaN;
}

int StatsQuantile::collapseFindFirstBuffer() {
    double nentries = countll();
    INVARIANT(nentries < 1e+14, "getQuantile will fail now.");
		 
    int first_buffer;
    for(first_buffer = nbuffers - 1;first_buffer > 0; --first_buffer) {
	if (buffer_level[first_buffer - 1] != buffer_level[first_buffer]) {
	    break;
	}
    }
    INVARIANT(first_buffer < nbuffers - 1,
	      "Whoa, collapse should always operate on at least two buffers");
    INVARIANT(buffer_level[first_buffer] >= 0,
	      "Whoa, buffer level should be positive");
    return first_buffer;
}

int64_t StatsQuantile::collapseSortBuffers(int first_buffer) {
    int64_t total_weight = 0;
    for(int i=first_buffer;i<nbuffers;i++) {
	total_weight += buffer_weight[i];
	INVARIANT(buffer_weight[i] > 0, "Whoa, buffer_weight should be at least 1");
	collapse_pos[i] = 0;
	if (buffer_sorted[i]) {
	    // could re-verify sortedness here, but we'll probably catch it later
	} else {
	    INVARIANT(i == (nbuffers - 1) || buffer_weight[i] == 1,
		      boost::format("Huh %d != %d // %d") % i 
		      % (nbuffers - 1) % buffer_weight[i]);
	    // sort the buffer
	    sort(all_buffers[i], all_buffers[i] + buffer_size, doubleOrder());
	    buffer_sorted[i] = true;
	} 
    }
    return total_weight;
}

int64_t StatsQuantile::collapseNextQuantileOffset(int64_t total_weight) {
    int64_t next_quantile_offset = 0;
    if ((total_weight % 2) == 0) {
	if (collapse_even_low) {
	    next_quantile_offset = total_weight / 2;
	    collapse_even_low = false;
	} else {
	    next_quantile_offset = (total_weight + 2) / 2;
	    collapse_even_low = true;
	}
    } else if ((total_weight % 2) == 1) {
	next_quantile_offset = (total_weight + 1) / 2;
    } else {
	FATAL_ERROR("Huh?");
    }
    return next_quantile_offset;
}

namespace {
    struct pairCmp {
	bool operator()(const pair<double, int> &a, const pair<double, int> &b) const {
	    return a.first >= b.first;
	}
    };
}

void StatsQuantile::collapse() {
    // could do a more "in-place" collapse by first doing the walk, and
    // for every entry which is not going to go into the final collation,
    // replace it with a NaN; then move all the entries in the output 
    // bucket which are not NaN's to the end, then collate again in order
    // filling into the output location; this does in-place, but requires
    // two walks over the larger data, and so may very well be slower
    // it's also a lot more complex

    int first_buffer = collapseFindFirstBuffer();

    // first, sort each of the unsorted input buffers 
    int64_t total_weight = collapseSortBuffers(first_buffer);
    
    int64_t next_quantile_offset = collapseNextQuantileOffset(total_weight);

    // Since we start counting at offset 0 (the paper starts at 1), we 
    // subtract one from the starting offset
    next_quantile_offset -= 1;
    DEBUG_SINVARIANT(next_quantile_offset >= 0);

    int64_t cur_quantile_offset = 0;
    int64_t next_output_pos = 0;
    double prev_val = -Double::Inf;

    // Because a double and int are small, it's faster to store the
    // pair rather than do the double dereference each time to
    // translate a buffer number into its value.
    PriorityQueue<pair<double, int>, pairCmp> pq(nbuffers);
    for(int i=first_buffer; i < nbuffers; ++i) {
	pq.push(make_pair(collapseVal(i), i));
    }

    while(next_output_pos < buffer_size) {
	int min_buffer = pq.top().second;
	double min_val = pq.top().first;

	DEBUG_INVARIANT(min_val >= prev_val, "Whoa, sort order error?!");
	prev_val = min_val;
	collapse_pos[min_buffer] += 1;
	// same logic as for output(), this entry is in positions
	// [cur_quantile_offset .. cur_quantile_offset + bw[mb] - 1]
	// so if after adding in bw[mb], cqo > nqo then we just passed
	// nqo, so it should be used as the value to fit into the
	// new list
	cur_quantile_offset += buffer_weight[min_buffer];
	if (cur_quantile_offset > next_quantile_offset) {
	    tmp_buffer[next_output_pos] = min_val;
	    next_output_pos += 1;
	    next_quantile_offset += total_weight;
	}

	if (collapse_pos[min_buffer] < buffer_size) {
	    pq.replaceTop(make_pair(collapseVal(min_buffer), min_buffer));
	} else {
	    pq.pop();
	    DEBUG_SINVARIANT(next_output_pos == buffer_size || !pq.empty());
	}
    }
    INVARIANT(next_quantile_offset / total_weight == buffer_size,
	      boost::format("Huh, didn't get to correct quantile"
			    " offset %lld/%d != %d?!")
	      % next_quantile_offset % total_weight % buffer_size);

    // update all_buffers[first_buffer]
    memcpy(all_buffers[first_buffer],tmp_buffer, buffer_size * sizeof(double));
    buffer_level[first_buffer] = buffer_level[first_buffer] + 1;
    buffer_weight[first_buffer] = total_weight;
    buffer_sorted[first_buffer] = true;

    // clean up buffer weights
    for(int i=first_buffer + 1;i<nbuffers;i++) {
	buffer_weight[i] = -1;
	buffer_level[i] = -1;
	buffer_sorted[i] = false;
    }
    cur_buffer = first_buffer + 1;
}

void StatsQuantile::dumpState() {
    printf("%d buffers, each containing %d entries\n",
	   nbuffers,buffer_size);
    printf("cur buffer is %d, cur buffer pos is %d\n",
	   cur_buffer,cur_buffer_pos);

    for(int i=0;i<=cur_buffer;i++) {
	cout << boost::format("  buffer %d, level %d, weight %d:\n    ")
	    % i % buffer_level[i] % buffer_weight[i];
	int max = i == cur_buffer ? cur_buffer_pos : buffer_size;
	for(int j=0;j<max;j++) {
	    printf("%.4g, ",all_buffers[i][j]);
	    if ((j%10) == 9) {
		printf("\n    ");
	    }
	}
	printf("\n");
    }
}

void StatsQuantile::printRome(int depth, ostream &out) const {
    Stats::printRome(depth,out);
    string spaces;
    for(int i = 0; i < depth; i++) {
	spaces += " ";
    }
    double nentries = countll();
    if (nentries > 0) {
	out << spaces << "{ quantiles (\n";
	double step = 1.0 / (double)print_nrange;
	int nquantiles = 0;
	for(double quantile = step;Double::lt(quantile,1.0);quantile += step) {
	    double quantile_value = getQuantile(quantile);
	    out << spaces << "  { " << quantile*100 << " " << quantile_value << " }\n";
	    ++nquantiles;
	}
	for(double tail_frac = 0.1; (tail_frac * nentries) >= 1.0;) {
	    double quantile_value = getQuantile(1-tail_frac);
	    out << spaces << "  { " << 100*(1-tail_frac) << " " << quantile_value << " }\n";
	    tail_frac /= 2.0;
	    quantile_value = getQuantile(1-tail_frac);
	    out << spaces << "  { " << 100*(1-tail_frac) << " " << quantile_value << " }\n";
	    tail_frac /= 5.0;
	}
	out << spaces << ") }\n";
    }
}

void StatsQuantile::printFile(FILE *out, int nranges) {
    ostringstream tmp;
    printTextRanges(tmp, nranges);
    fwrite(tmp.str().data(), tmp.str().size(), 1, out);
}

void StatsQuantile::printTextRanges(ostream &out, int nranges, double multiplier) const {
    nranges = (nranges == -1 ? print_nrange : nranges);
    out << boost::format("%lld data points, mean %.6g +- %.6g [%.6g,%.6g]\n")
	% countll() % (multiplier * mean()) % (multiplier * stddev()) 
	% (multiplier * min()) % (multiplier * max());
    if (countll() == 0) return;
    out << boost::format("    quantiles about every %.0f data points:")
	% ((double)countll()/(double)nranges);
    double step = 1.0 / (double)nranges;
    int nquantiles = 0;
    for(double quantile = step;Double::lt(quantile,1.0);quantile += step) {
	if ((nquantiles % 10) == 0) {
	    out << boost::format("\n    %.4g%%: ") % (quantile * 100);
	} else {
	    out << ", ";
	}

	out << boost::format("%.8g") % (multiplier * getQuantile(quantile));
	++nquantiles;
    }
    out << "\n";
}

void StatsQuantile::printTail(FILE *out) {
    ostringstream tmp;
    printTextTail(tmp);
    fwrite(tmp.str().data(), tmp.str().size(), 1, out);
}

// TODO: should this print tails if we have very few data points,
// e.g. 22 data points, should we get the 90%,95% tails?  This happens
// in the dataseries groupby regression test, but may be otherwise
// irrelevant.
void StatsQuantile::printTextTail(ostream &out, double multiplier) const {
    double nentries = countll();
    out << "  tails: ";
    for(double tail_frac = 0.1; (tail_frac * nentries) >= 10.0;) {
	if (tail_frac < 0.05) {
	    out << ", ";
	}
	out << boost::format("%.12g%%: %.8g")
	    % (100*(1-tail_frac)) % (multiplier * getQuantile(1-tail_frac));
	tail_frac /= 2.0;
	out << boost::format(", %.12g%%: %.8g")
	    % (100*(1-tail_frac)) % (multiplier * getQuantile(1-tail_frac));
	tail_frac /= 5.0;
    }
    out << "\n";
}

void StatsQuantile::printText(ostream &out) const {
    printTextRanges(out);
    printTextTail(out);
}

size_t StatsQuantile::memoryUsage() const {
    // primary data (init_buffers) + secondary metadata (init)
    return nbuffers * buffer_size * sizeof(double) // primary
	+ nbuffers * (sizeof(int64_t) + sizeof(int) + sizeof(bool)
		      + sizeof(double) + sizeof(int)) // secondary
	+ sizeof(StatsQuantile);
}
