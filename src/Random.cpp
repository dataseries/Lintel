/* -*-C++-*-
   (c) Copyright 1994-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Class for random-number generation
*/

#include <time.h>
#include <sys/time.h>

#include <Lintel/Double.hpp>
#include <Lintel/AssertBoost.hpp>
#include <Lintel/Random.hpp>


//////////////////////////////////////////////////////////////////////////////
// RandSeed class
//////////////////////////////////////////////////////////////////////////////

void RandSeed::reset()
{
  reset(RandSeederDefault);
};

void RandSeed::reset(long s)
{
  seeds[0] = 0;
  seeds[1] = (unsigned short) ((s>>16) & 0xffff);
  seeds[2] = (unsigned short)       (s & 0xffff);
};

void RandSeed::reset(RandSeed &s)
{
  seeds[0] = s.seeds[0];
  seeds[1] = s.seeds[1];
  seeds[2] = s.seeds[2];
};

void RandSeed::reset(unsigned short s1, unsigned short s2, unsigned short s3)
{
  seeds[0] = s1;
  seeds[1] = s2;
  seeds[3] = s3;
};


// Note: this function cannot be in the .H file because it makes a
// forward reference to class RandSeeder.
//
void RandSeed::reset(RandSeeder* sd)
{
  if (sd == NULL)
    use_time_of_day();		// calculate a seed from the time of day
  else
    sd->next_seed(seeds);	// ask the seeder for a new seed
};


void RandSeed::use_time_of_day()
{
    struct timeval t;
    struct timezone z;
    gettimeofday(&t,&z);

    seeds[0] = (unsigned short) (t.tv_sec&0xffff);
    seeds[1] = (unsigned short) (t.tv_usec>>16);
    seeds[2] = (unsigned short) (t.tv_usec&0xffff);
};



//////////////////////////////////////////////////////////////////////////////
// RandSeeder class
//////////////////////////////////////////////////////////////////////////////

// Initialize from time-of-day (no args) or explicit seed (a long)

RandSeeder::RandSeeder()       : sr(new RandintU())  {};
RandSeeder::RandSeeder(long s) : sr(new RandintU(s)) {};


// Reset the seed value, either from the time-of-day or to a given value

void RandSeeder::reset()		{ sr->seed.reset();  };
void RandSeeder::reset(long s)		{ sr->seed.reset(s); };
void RandSeeder::reset(RandSeed& s)	{ sr->seed.reset(s); };
void RandSeeder::reset(unsigned short s1, unsigned short s2, unsigned short s3)
					{ sr->seed.reset(s1,s2,s3); };
void RandSeeder::reset(RandSeeder* sd)	{ sr->seed.reset(sd); };


// Create a new 48 bit seed by taking the high order bit of each of 48 
// calls to randint and concatenating them.  Relies on randint returning
// a 32-bit long, the leftmost bit is the sign bit, the second-to-leftmost
// is the high order bit.
    
void RandSeeder::next_seed(unsigned short* newseed)
{
  newseed[0] = 0;		// initialize to all 0's
  newseed[1] = 0;		// was: RandSeed newseed = RandSeed(0);
  newseed[2] = 0;

  for (int j = 0; j < 3; j++) {
    for (int i = 0; i < 16; i++) {
      newseed[j] = (unsigned short)
       (newseed[j] | ((unsigned short) ((sr->draw() & 0x40000000) >> (30-i))));
    }
  }
  return;
};


//----------------------------------------------------------------
// Global default seed value: this is set to a deterministic seed
// value so that runs will be repeatable by default.
//----------------------------------------------------------------
RandSeeder *RandSeederDefault = new RandSeeder(7);




//////////////////////////////////////////////////////////////////////////////
// Rand class
//////////////////////////////////////////////////////////////////////////////

// is empty



//////////////////////////////////////////////////////////////////////////////
// RandE class
//////////////////////////////////////////////////////////////////////////////

double RandE::draw()
{
  return -mean_value * log(1.0-erand48(seed.seeds));
}



//////////////////////////////////////////////////////////////////////////////
// RandG class
//////////////////////////////////////////////////////////////////////////////

RandG::RandG()
  : Rand(), seed(), mu(1), sigma(1)
{
  initialise();
};
  

RandG::RandG(double mu_, double sigma_)
  : Rand(), seed(), mu(mu_), sigma(sigma_)
{
  initialise();
};


RandG::RandG(double mu_, double sigma_, RandSeeder *sd)
  : Rand(), seed(sd), mu(mu_), sigma(sigma_)
{
    initialise();
};


void RandG::initialise()
{
  alpha = mu*mu/(sigma*sigma);
  if (alpha > 0.25) {
    max_random = 2147483648.0;			// i.e. 2^31 
    inv_max_random = 4.656612873077e-10;	// i.e. 2^-31
    a = alpha - 0.25;
    b = alpha/a;
    c = 2.0/a;
    d = c + 2.0;
    t = sqrt(1.0/alpha);
    h1 = (0.4417 +0.0245 * t/alpha) * t;
    h2 = (0.222  -0.043  * t) * t * max_random;
  }
}
    

// Erlang random variable generator for 0< alpha < 1 
// Ahrens and Dieter, Computing, 1972 

double RandG::gs() 
{
  double b, p, x, result;
  b = 1.0 + alpha/2.71828;
  while (1) {
    p = b * erand48(seed.seeds);
    if (p <= 1.0) {
      x = exp( log(p)/alpha );
      if (erand48(seed.seeds) <= exp(-x)) {
	result = x/alpha;
	break;
      }
      else {
	x =  -log((b-p)/alpha);
	if (log(erand48(seed.seeds)) <= (alpha -1.0)* log(x)) {
	  result = x/alpha;
	  break;
	}
      }
    }
  }
  return result;
}



// Erlang random variable generator for alpha > 0.25 
// Cheng and Feast (CACM, 1980), slightly optimized.

double RandG::gbh() 
{
  double u, u1, u2, w;
  do {
    do {
      u1 = nrand48(seed.seeds);
      u2 = u1 +h1*nrand48(seed.seeds)-h2;
    }
    while ((u2 <= 0.0) || (u2 >=max_random));
    w = u1/u2;
    u2 *= inv_max_random;
    w *= w;
    w *= w;
    u  = w;
    w *= b;
  } while ( ((c*u2-d+w+1.0/w) > 0.0) && ((c*log(u2) -log(w)+w-1.0) >= 0.0) );
  return u;
}



double RandG::draw()
{
  return (alpha < 0.25) ? mu*gs() : mu*gbh();
}






//////////////////////////////////////////////////////////////////////////////
// RandGeom 
//////////////////////////////////////////////////////////////////////////////

// **note**: the use of drand48 below abrogates our seeds - it 
// **note**: essentially ignores them.  This needs fixing ...
double RandGeom::draw() {
  return (double) ceil( log(drand48()) / log(1.0 - 1.0/mean_value) );
}




//////////////////////////////////////////////////////////////////////////////
// RandH class
//////////////////////////////////////////////////////////////////////////////


const double RandH::eps = 1.e-10; // Slop on floating-point equality tests.

//--------------------------------------------------------------
// Constructors
//
// Note:  we make our tables one element larger than the input
// arrays to build an extra bin at the beginning of the table.
//--------------------------------------------------------------

RandH::RandH(unsigned       array_size_in, 
             const double *x_in, 
	     const double *y_in)
  : Rand(),
    table_size(array_size_in + 1),
    randForSearch(new RandU(0.0, 1.0))
{
    init(array_size_in, x_in, y_in);
}


RandH::RandH(unsigned       array_size_in, 
             const double *x_in, 
	     const double *y_in,
	     RandSeeder   *seed_in)
  : Rand(),
    table_size(array_size_in + 1),
    randForSearch(new RandU(0.0, 1.0, seed_in))
{
    init(array_size_in, x_in, y_in);
}


RandH::RandH(const RandH& rand_in)
    : Rand(),
      table_size(rand_in.table_size),
      randForSearch(new RandU(0.0, 1.0))
{
    init(rand_in.table_size - 1, rand_in.x_table + 1, rand_in.y_table + 1);
}



//--------------------------------------------------------------
// Error checking and creating the tables 
//   1. sum of the index_array must be 1 (with rounding error).
//   2. the last element of the value_array must be 1.  
//   3. the value_array must be in ascend order.  
// Checks that look down the entire array need only be  performed
// with higher AssertLevels.
// If all is ok, we create the value_table and the index_table.
//--------------------------------------------------------------

void RandH::init(const unsigned input_array_size,
		 const double *x_in, 
		 const double *y_in)
{
  const double epsilon = 1e-10;	// allowable rounding error on 
				// the "sums to 1.0" test

  x_table = new double[table_size]; // copy of what is passed in
  y_table = new double[table_size]; // cumulative histogram of input

  // Initialize the first element in each table
  x_table[0] = 0;
  y_table[0] = 0;

  // Calculate the mean by integrating: it's the sum of the centers
  // of each bin, times their probability.
  mean_value = 0.0;

  // Copy the x_table and y_table from the x_in and y_in arrays,
  // checking that their entries are in ascending order.
  for (unsigned i=1; i < input_array_size+1; i++) {
    x_table[i] = x_in[i-1];
    y_table[i] = y_in[i-1];
 
    //DebugMsg(10, ("x_table[%d] = %lf, y_table[%d] = %lf\n",
    //	  i, x_table[i], i, y_table[i]));
    DEBUG_SINVARIANT((x_table[i-1] <= x_table[i] + epsilon));
    DEBUG_SINVARIANT((y_table[i-1] <= y_table[i] + epsilon));
    mean_value += (x_table[i]+x_table[i-1])/2. * 
                  (y_table[i]-y_table[i-1]);
  }

  // The last element in y_array should be ~1.0
  double last_y = y_table[input_array_size];
  DEBUG_SINVARIANT(1.0-epsilon <= last_y  &&  last_y <= 1.0+epsilon);
}


//--------------------------------------------------------------
// destructor 
//--------------------------------------------------------------

RandH::~RandH()
{
    delete [] y_table;
    delete [] x_table;
}


//--------------------------------------------------------------
// Draw a number from RandH distribution.
//
// - draw a uniform random number
// - search the index table to find the region that covers 
//   the random number and save the low and high index of 
//   the region
// - find the value for the low and high boundaries of the region
//   in both the x_table and the y_table
// - use linear interpolation to get the value from the distribution
//   (with the boundary values and the random number)
//      a = a0 + slope * (b - b0);
//--------------------------------------------------------------

double RandH::draw()
{
  // First draw a random number: this will be used as a "value (y)", and
  // the tables used to map this across to an "index (x)", which is what
  // we return.

  double rand = randForSearch->draw();

  unsigned low_index  = find_region(0, (table_size - 1), rand);
  unsigned high_index = low_index + 1;

  double x0 = x_table[low_index];
  double delta_y = y_table[high_index] - y_table[low_index];
  double delta_x = x_table[high_index] - x_table[low_index];
  double slope = delta_x / delta_y; // we are interpolating X-values, not Y

  return x0 + slope * (rand - y_table[low_index]);
}



double RandH::draw(double _low, double _high)
{
  // First draw a random number: this will be used as a "value (y)", and
  // the tables used to map this across to an "index (x)", which is what
  // we return.
    zero_range = false;

    DEBUG_SINVARIANT(_low >= 0);
    DEBUG_SINVARIANT(_high <= x_table[(table_size - 1)]);
    DEBUG_SINVARIANT(_low <= _high);

    // These should be replaced with a binary search.
    int i;
    double low_x, low_y, high_x, high_y;
    for (i = 0;  _low > x_table[i]; i++);
    if (i == 0)
	{
	    low_x = 0;
	    low_y = 0;
	}
    else
	{
	    low_x = x_table[i - 1];
	    low_y = y_table[i - 1];
	}
    high_x = x_table[i];
    high_y = x_table[i];

    double slope = (high_y - low_y) / ((double)
		    (high_x - low_x));
    double low_cdf = low_y + slope*(_low - low_x);

    for(; _high > x_table[i]; i++);
 if (i == 0)
	{
	    low_x = 0;
	    low_y = 0;
	}
    else
	{
	    low_x = x_table[i - 1];
	    low_y = y_table[i - 1];
	}
    high_x = x_table[i];
    high_y = x_table[i];

    
    slope = (high_y - low_y) / 
		    (high_x - low_x);
    double high_cdf = low_y + slope*(_low - low_x);

  double rand = randForSearch->draw();

  // If the probability of drawing a number in the specified reigon
  // is zero, then we need to return something else
  if (high_cdf == low_cdf)
      {
	  zero_range = true;
	  return rand;
      }
  rand = rand*(high_cdf - low_cdf) + low_cdf;

  unsigned low_index  = find_region(0, (table_size - 1), rand);
  unsigned high_index = low_index + 1;

  double x0 = x_table[low_index];
  double delta_y = y_table[high_index] - y_table[low_index];
  double delta_x = x_table[high_index] - x_table[low_index];
  slope = delta_x / delta_y; // we are interpolating X-values, not Y

  double ans = x0 + slope * (rand - y_table[low_index]);
  if (ans < _low) return _low;
  if (ans > _high) return _high;
  return ans;

}



//--------------------------------------------------------------
// Use binary search to find the sub region in the value array 
// that covers the value. 
//
// - if the difference between the high bound and the low bound 
//     is one and the value is in the region, the search is 
//     over--return the l_index.
//
// - if not, divide the table in half. If the value is bigger 
//     than or equal to the table value at the middle point, 
//     search the higher half of the table; else search the 
//     lower half of the table. 
//
// - repeat the above until the stopping condition is 
//     reached 
//--------------------------------------------------------------

unsigned 
RandH::find_region(unsigned l_index,	// low boundary of the region searched
		   unsigned h_index,	// high boundary of the region searched
		   double   value) 	// the value to find the bin for
{
  // Low index must be smaller than high index, and 
  // value can't be bigger than the biggest value in the y_table 
  DEBUG_SINVARIANT(l_index < h_index);
  DEBUG_SINVARIANT(value <= y_table[h_index]);

  if (value == y_table[h_index]) {	// Special case: very top of table
    return h_index - 1;
  }

  if (h_index == l_index + 1 &&		// Termination condition
      value >= y_table[l_index] &&
      value <  y_table[h_index]) {
    return l_index;
  }
    
  unsigned mid = (l_index + h_index)/2;	// Binary chop
  if (value >= y_table[mid]) {
    return find_region(mid, h_index, value);
  } else {
    return find_region(l_index, mid, value);
  }
}


//---------------------------------------------------------------------------
//
// Creates a RandH object with the discrete distribution specified in the 
// file.  See comment in Random.H for input format.
//
// ---------------------------------------------------------------------------

RandH* RandH::DiscreteRandom(std::string filename)
{
    // Try to open the file:
    FILE* f = fopen(filename.c_str(), "r");
    INVARIANT(f != NULL,
	      boost::format("Can't open histogram file %s: %s.")
	      % filename % strerror(errno));

    // Read the number of entries.  It must be at least two, otherwise
    // this histogram would be just a fixed value.
    int n;
    int n_scan = fscanf(f, "%d\n", &n);
    INVARIANT(n_scan == 1 && n > 1,
	      boost::format("First line of histogram file %s was not "
			    "a positive number.")
	      % filename);

    // Read the entries themselves; verify that none of the X or Y values
    // are out of range.
    double* raw_x = new double[n];
    double* raw_p = new double[n];
    double total_p = 0.0;

    for (int i=0; i<n; ++i) {
        n_scan = fscanf(f, "%lf %lf\n", &raw_x[i], &raw_p[i]);
        INVARIANT(n_scan == 2,
		 boost::format("Line %d of histogram file %s was not X and Y.")
		  % (i+2) % filename);
        INVARIANT(raw_x[i] >= 0.0,
		  boost::format("Histogram file %s X[%d]=%f is negative.")
		  % filename % i % raw_x[i]);
        INVARIANT(raw_p[i] >= 0.0,
		boost::format("Histogram file %s Prob[%d]=%f is not positive.")
		  % filename % i % raw_p[i]);
        total_p += raw_p[i];
    }
    fclose(f);

    // The total probability has to be non-zero:
    INVARIANT(total_p >= 0.0,
	      boost::format("Histogram file %s is all zero probabilities.")
	      % filename);
    // Rescale all the probabilites:
    for (int i=0; i<n; ++i) {
        raw_p[i] /= total_p;
    }

    // Verify that the X values are strictly ascending, with at least
    // a gap of 1.  Do not remove that check, because if two values
    // are within 0.1 of each other, the logic of the loop below that
    // converts this PDF into a CDF has to change radically.
    for (int i=1; i<n; ++i) {
        INVARIANT(raw_x[i] - raw_x[i-1] >= 1.0-eps,
	     boost::format("Histogram %s X[%d]=%f and X[%d]=%f are too close.")
		  % filename.c_str() % (i-1) % raw_x[i-1] % i % raw_x[i]);
    }

    // Now convert the probabilities into a CDF.  This requires adding
    // extra X values, namely after each. So we allocate space for an
    // array 2n.

    // Here are a few examples of how this conversion works:

    // Before x:   200         400         800
    // Before p:   0.5         0.3         0.2
    // After  x:   200.0 200.1 400.0 400.1 800.0 800.1
    // After  p:   0.0   0.5   0.5   0.8   0.8   1.0

    // Before x:   0       1       2       4       7
    // Before p:   0.1     0.1     0.3     0.2     0.3
    // After  x:       0.1 1.0 1.1 2.0 2.1 4.0 4.1 7   7.1
    // After  p:       0.1 0.1 0.2 0.2 0.5 0.5 0.7 0.7 1.0

    double* x = new double[2*n];
    double* p = new double[2*n];
    int u = 0; // Where the next entry will go.
    double cdf = 0.0; // How much probability we have accumulated so far.

    for (int i=0; i<n; ++i) {
        // We always emit the current value, but with the previous CDF.
        // Only exception: If this is the first value and it is zero.
        if (i>0 || raw_x[i] > eps) {
            x[u] = raw_x[i];
            p[u] = cdf;
            ++u;
        }

        // Now we can add the current probability to the CDF:
        cdf += raw_p[i];

        double end_this_bin = raw_x[i] + 0.1;
        x[u] = end_this_bin;
        p[u] = cdf;
        ++u;
    }

    // Make sure no more than n+1 values were used:
    INVARIANT(u<=2*n, boost::format("Histogram %s internal error: %d<=2*%d")
	      % filename % u % n);

    // Logic check: The very last value must be 1:
    INVARIANT(p[u-1] >= 1.0 - eps,
	      boost::format("Histogram %s internal error: Sum is %f.")
	      % filename % p[u-1]);

    // Create the desired random distribution:
    RandH* return_value = new RandH(u, x, p);

    // Clean up and return:
    delete[] raw_x;
    delete[] raw_p;
    delete[] x;
    delete[] p;

    return return_value;
}

//---------------------------------------------------------------------------
//
// Creates a RandH object with the distribution specified in the 
// file.  See comment in Random.H for input format.
//
// ---------------------------------------------------------------------------
RandH* RandH::HistogramRandom(std::string filename)
{
    // Try to open the file:
    FILE* f = fopen(filename.c_str(), "r");
    INVARIANT(f != NULL,
	      boost::format("Can't open histogram file %s: %s.")
	      % filename % strerror(errno));

    // Read the number of entries.  It must be at least two, otherwise
    // this histogram would be just one bin, which is a uniform
    // distribution.
    int n;
    int n_scan = fscanf(f, "%d\n", &n);
    INVARIANT(n_scan == 1 && n > 1,
	      boost::format("First line of histogram file %s was not "
			    "a positive number.")
	      % filename);

    // Read the entries themselves; verify that none of the X or Y values
    // are out of range.
    double* x = new double[n];
    double* p = new double[n];
    double total_p = 0.0;

    for (int i=0; i<n; ++i) {
        n_scan = fscanf(f, "%lf %lf\n", &x[i], &p[i]);
        INVARIANT(n_scan == 2,
		 boost::format("Line %d of histogram file %s was not X and Y.")
		  % (i+2) % filename);
        INVARIANT(x[i] >= 0.0,
		  boost::format("Histogram file %s X[%d]=%f is negative.")
		  % filename % i % x[i]);
        INVARIANT(p[i] >= 0.0,
	        boost::format("Histogram file %s Prob[%d]=%f is not positive.")
		  % filename % i % p[i]);
        total_p += p[i];
    }
    fclose(f);

    // The total probability has to be non-zero:
    INVARIANT(total_p >= 0.0,
	      boost::format("Histogram file %s is all zero probabilities.")
	      % filename);

    // Rescale all the probabilites, and integrate them into a CDF:
    double cdf = 0;
    for (int i=0; i<n; ++i) {
        cdf += p[i] / total_p;
        p[i] = cdf;
    }

    // Create the desired random distribution:
    RandH* return_value = new RandH(n, x, p);

    // Clean up and return:
    delete[] x;
    delete[] p;

    return return_value;
}



//////////////////////////////////////////////////////////////////////////////
// RandN class
//////////////////////////////////////////////////////////////////////////////

//--Default seed value

RandN::RandN()			// Default mean/variance
  : Rand(), seed(), mu(0.0), sigma(1.0)
{};

RandN::RandN(double mu_, double sigma_)
  : Rand(), seed(), mu(mu_), sigma(sigma_)
{};


//--Explicit seed value

RandN::RandN(double mu_, double sigma_, RandSeeder *sd)
  : Rand(), seed(sd), mu(mu_), sigma(sigma_)
{};


// %%% Someone should check how this algorithm works.  It uses
// %%% two random numbers to generator one normally distributed
// %%% random number.  I think the reason is that it could as well
// %%% generate a pair of random numbers once it has drawn two
// %%% random numbers, but that would be more work.
double RandN::draw() 
{
  double temp = -2.0*log(erand48(seed.seeds));
  DEBUG_SINVARIANT(temp > 0.0);
  return mu + sigma * sqrt(temp) * cos(2.0*M_PI*erand48(seed.seeds));
};




//////////////////////////////////////////////////////////////////////////////
// RandU class
//////////////////////////////////////////////////////////////////////////////

// is empty


//////////////////////////////////////////////////////////////////////////////
// Randint{,U} classes
//////////////////////////////////////////////////////////////////////////////

// are empty


//////////////////////////////////////////////////////////////////////////////
// RandList class
//////////////////////////////////////////////////////////////////////////////

void RandList::init(const char* file_name)
{
    double temp;
    std::ifstream input(file_name);
    INVARIANT(input.good(), 
	      boost::format("Could not open RandList input file \"%s\"")
	      % file_name);
    
    while(input >> temp) objects.push_back(temp);
    
    if (objects.size() < 1)
	{
	    std::cerr << "WARNING!  RandList(" << file_name 
		      << ") is an empty list!" << std::endl;
	}
}

// This constructor takes a double* instead of a std::vector<double>
// to remain consistent with the RandH constructors.
void RandList::init(const double* input, int size)
{
    int x = 0;
    while(x < size) objects.push_back(input[x++]);
    
    if (objects.size() < 1)
	{
	    std::cerr << "WARNING!  RandList(const double*) is an empty list!" 
		      << std::endl;
	}
}



double RandList::draw() 
{ 
    INVARIANT(objects.size() >= 1, 
	      "Tried to draw() from an empty RandList");
    current %= objects.size(); return objects[current++]; 
}

double RandList::mean()
{

    if (objects.size() == 0)
	return 0;
    double sum = 0;
    int x;
    for (x = 0; x < (int)objects.size(); x++)
	{
	    sum += objects[current++];
	}
    return sum / objects.size();
}





