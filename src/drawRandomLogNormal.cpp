/*
   (c) Copyright 1998-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

// This program does the following tasks:
// - given a desired mean, sigma, and upper limit
// - it fits a truncated log-normal distribution which has these
//   parameters, if one exists
// - it then generates a random number distributed according to
//   that distribution, if given a random integer.
// Note that not all parameters lead to a solution; for example,
// a log-normal distribution with a mean of 1, upper limit of 2,
// and sigma of 1000 can obviously not exist.  If no solution exists,
// this program prints an error message to stderr, and exits with
// a non-zero status code.

// If you don't specify a random integer, no random number will 
// be generated.  If you use the option -v, the (input and fitted) 
// parameters will be output to stderr.

// This program contains all the routines which are needed, namely
// - logNormal, which calculates the PDF (probability distribution
//   function) of a log normal distribution.
// - momentLogNormal, which calculates (analytically) the moments
//   of a log-normal distribution,
// - findZero, a general purpose multi-dimensional equation solver, using
//   the Newton-Raphson method,
// - solve2, a tiny routine to solve a linear system of 2 equations
//   with two unknowns,
// - solveLogNormal, which uses many of the above to solve the 
//   parameters of the log normal distribution, given the desired
//   mean and sigma,
// - normalRandomLimit, a random number generator which generates
//   random numbers with a gaussian distribution with an upper limit,
// - momentDerivMean and momentDerivSigma, which calculate (analytically)
//   the two derivatives of the moments, used only for unit test.

// To test this program, define REGRESSION_TEST to be
// - 10 to test the moments of the log-normal distribution
// - 20 to test the derivatives of these moments.
// - 30 to 32 to test the equation solver: 30 tries linear equations,
//   31 and 32 try non-linear ones, 32 is even transcedental.
// - 40 to test solveLogNormal
// - 50 to generate a histogram of random numbers
// Then recompile and run the resulting executable.

// TODO: generalize this, fix up the regression test approach, improve the documentation.
// Probably rename to the standard lowercase convention for commands.

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>

/*

=pod

=head1 NAME

drawRandomLogNormal - a program to calculate truncated log-normal distributions.

=head1 SYNOPSIS

 % drawRandomLogNormal [-v] mean std_dev up_limit [rand_int]

=head1 DESCRIPTION

Generates a random number, distributed with a truncated log-normal distribution. You specify
desired mean, std. deviation, upper limit, and a random integer.  The random number is printed on
stdout.  If the parameters make no sense, an error message is printed on stderr.  Option -v also
outputs the parameters of the distribution on stderr.

=cut

 */


const double s2 = sqrt(2.);

// ========================================================================
// PDF (probability distribution) of a truncated log-normal.
// Correctly normalized.  
// Input: log_mean, log_sigma, and lin_up_limit are the parameters of
//        the distribution.
double logNormal(double x, 
                 double log_mean, double log_sigma, double lin_up_limit)
{
    // Avoid log(0) errors:
    if (x < log_mean * 1.e-6)
        return 0.0;

    if (x >= lin_up_limit)
        return 0.0;

    // Normalization:
    double normal = ( 1. + 
                      erf( (log(lin_up_limit)-log_mean) /s2/log_sigma)) 
                    /2.;

    return exp(-(log(x)-log_mean)*(log(x)-log_mean)/2./log_sigma/log_sigma) /
           sqrt(2.*M_PI)/log_sigma /x /normal;
}

// ======================================================================
// Routine to calculate the moments of the truncated log-normal
// distribution.  The analytic formulae came from Arif.
// Power is the rank of the moment to calculate.  Power=0 gives
// you the total integral (should be ==1), power=1 gives you the mean,
// power=2 allows calculating the standard deviation.

// To test it, define REGRESSION_TEST = 10.  The regression test
// generates a new main() program, which numerically integrates the
// moments, and compares the numeric results with the analytic version.

double momentLogNormal(double log_mean, double log_sigma, 
                       double up_limit, int power)
{
    return exp(power*log_mean +
               power*power*log_sigma*log_sigma / 2.) *
           (1. + erf((up_limit - log_mean - power*log_sigma*log_sigma) 
                 /s2/log_sigma)) /
           (1. + erf( (up_limit - log_mean) / s2/log_sigma));
}

// ======================================================================
// findZero: Routine which solves a multi-dimensional non-linear system of
// equations: You have N equations with N unknowns.  This routine will
// solve f(x) = 0, where both f and x are N-dimensional vectors.
// Internally, it needs a matrix inversion routine.  This version
// uses a very simple matrix inversion (solve2), which is limited
// to dimension 2 (two equations, two unknowns).
// This function is adapted from mnewt, Numerical Recipes in C,
// 2nd edition, P. 381

// You have to provide a function which calculates the goal functions
// f(x), plus the jacobian matrix (the matrix of derivatives of all
// f(x) w.r.t. all x).

// To test this equation solver, compile with REGRESSION_TEST = 31
// or 32.  Regression test 30 tests just the linear equation solver.

// Type definition of the user-supplied function: Calculate goal 
// functions f[] of parameters x[], and jacobian matrix j[][].
// Arrays f[] and x[] each have n parameters. j[][] is a n*n matrix.
// The array is expressed as an array of arrays (pointer to pointer).
// j[i][j] is df[i] / dx[j].
// The caller has already allocated the arrays.
typedef void (multiFunc)(int n, double const x[], double f[], double* j[]);

// The original program (in the numerical recipes book) uses LU-decomposition
// for inverting the jacobian matrix (using routines ludcmp and lubksb).
// This is overkill for a 2-dimensional problem.
// This much simpler routine solves any 2*2 linear equation: ax+by=p and 
// cx+dy=q.
// Parameters:
// - a the array, in the form a[0][0]=a, a[0][1]=b, a[1][0]=c, a[1][1]=d
// - b the right-hand side, with b[0]=p, b[1]=q
// - d (returned) the determinant
// - x (returned) the solution
// The return value indicates whether a solution was found (true=yes)
bool solve2(double *const *const a, double *const b, double& d, double * x)
{
    d = a[0][0] * a[1][1] - a[0][1] * a[1][0];

    bool return_value;
    if (fabs(d) < 1.e-6) {
        x[0] = 0.0;
        x[1] = 0.0;
        return_value = false;
    }
    else {
        x[0] = ( b[0] * a[1][1] - b[1] * a[0][1] ) / d;
        x[1] = ( b[1] * a[0][0] - b[0] * a[1][0] ) / d;
        return_value = true;
    }

    return return_value;
}

// findZero: Routine which solves a multi-dimensional non-linear system of
// equations. Described above.  Parameters:
// n: Number of dimensions. Must be two, because we currently use a
//    2*2 matrix inversion routine.
// f: Function to calculate functions and jacobian
// x[]: Initial guess of solution (in), final solution (out)
// n_trial: number of trials (max number of times the user function is to 
// be evaluated)
// tolx: Tolerance in parameter-space
// tolf: Tolerance in solution space
// Return value is success indicator (true=zero found, false=too many trials)
bool findZero(int n, multiFunc func, double x[], int n_trial, 
              double tolx, double tolf) 
{
    if (n!=2) {    
        fprintf(stderr, "findZero only works for n=2, not for n=%d.", n);
        exit(1);
    }

    // Allocate all arrays.
    // WARNING: Do not add a return to the middle of this function, as
    // this will cause a memory leak.  Instead, exit by going to 
    // findZero_exit (Knuth goto)
    bool return_value = false;
    double* f = new double[n];
    double* f_minus = new double[n];
    double* c = new double[n];
    // Why oh why does C++ not have native multi-dimensional arrays?
    double** j = new double*[n];
    for (int i=0; i<n; ++i)
        j[i] = new double[n];

    for (int i_trial=0; i_trial<n_trial; ++i_trial) {
        // Calculate function and derivatives at current point:
        func(n, x, f, j);

        // If we are close enough in function space, exit:
        double err_f = 0;
        for (int i=0; i<n; ++i)
            err_f += fabs(f[i]);
        if (err_f <= tolf) {
            return_value = true;
            goto findZero_exit;
        }

        // Calculate the Newton-Raphson correction, by solving j * c = -f:
        for (int i=0; i<n; ++i) 
            f_minus[i] = -f[i];
        double determinant;
        bool solve_worked = solve2(j, f_minus, determinant, c);
        // The determinant of the jacobian can vanish.  In that case,
        // we are dead (probably sitting right on a maximum of the
        // function).
        if (!solve_worked)
            goto findZero_exit;

        // Move the current point by the correction, and measure how much
        // we moved:
        double errx = 0.0;
        for (int i=0; i<n; ++i) {
            errx += fabs(c[i]);
            x[i] += c[i];
        }

        // If we stopped moving, we are done:
        if (errx <= tolx) {
            return_value = true;
            goto findZero_exit;
        }
    }

findZero_exit:
    delete[] f;
    for (int i=0; i<n; ++i)
        delete[] j[i];
    delete[] j;
    delete[] f_minus;
    delete[] c;

    return return_value;
}

//======================================================================
#if defined(REGRESSION_TEST) && ( REGRESSION_TEST==20 || REGRESSION_TEST==40)
// Routines to calculate the derivative of the moments of a truncated
// log-normal, derived w.r.t. log_mean and log_sigma (one routine
// for each).  The derivatives were calculated analytically.
// NOTE: Even though these derivatives are analytical, they are
// numerically very unstable. They are used only in their own unit tester,
// and in the unit tester for the log-normal solver

// To test the derivatives, compile with REGRESSION_TEST = 20. This 
// adds a new main to the program.

// For conciseness, we use m, s, b and r for the parameters here.
const double epsilon = 2. / sqrt(M_PI);

double expmx2(double x)
{
    return exp(-x*x);
}

#if 0
// The following derivatives were hand-calculated.  They work most of the
// time (actually, more often than the ones calculated with mathematica), 
// but once in a while they give numerically very wrong answers.
// They probably rely on gratuitous cancellation between terms.
double term_1(double m, double s, double /*b*/, int r)
{
    return exp(r*m + r*r*s*s / 2.);
}

double term_2(double m, double s, double b, int r)
{
    return 1. + erf((b - m - r*s*s) /s2/s);
}

double term_3(double m, double s, double b, int /* r */)
{
    return 1. + erf( (b - m) / s2/s);
}

// Derivative of moment w.r.t. mean. 
double momentDerivMean(double m, double s, double b, int r)
{
    return r * momentLogNormal(m,s,b,r)
           - term_1(m,s,b,r) * term_3(m,s,b,r) * epsilon / s2 / s *
             expmx2( (b-m-r*s*s)/s2/s)
           - term_1(m,s,b,r) * term_2(m,s,b,r) * epsilon / 2. / s *
             expmx2( (b-m)/s2/s);
}

// Derivative of moment w.r.t. sigma.
double momentDerivSigma(double m, double s, double b, int r)
{
    return r*r * s * momentLogNormal(m,s,b,r)
           + term_1(m,s,b,r) * term_3(m,s,b,r) * epsilon *
             ( (b-m) /s2/s/s - r/s2 ) *  
             expmx2( (b-m-r*s*s)/s2/s)
           - term_1(m,s,b,r) * term_2(m,s,b,r) * epsilon *
             ( b-m ) /s2/s/s *
             expmx2( (b-m)/s2/s);
}

#else 
// The following derivatives were calculated by Arif with Mathematica.
// They are just as inaccurate:
double momentDerivMean(double m, double s, double b, int r)
{
    if (r==1)
        return 
            ( exp(m + s*s/2.) *
              ( -epsilon * (1. + erf((b - m)/s2/s)) *
                  expmx2((-b+m+s*s)/s2/s) *s +
                epsilon * (1. - erf((-b + m + s*s)/s2/s)) *
                  expmx2((b - m)/s2/s) *s +
                (1. + erf((b - m)/s2/s)) *
                (1. - erf((-b + m + s*s)/s2/s)))) /
            ( 1. + erf((b - m)/s2/s)) /
            ( 1. + erf((b - m)/s2/s));

    else if (r==2)
        return
             (exp(2.*(m + s*s)) *
             (-((epsilon*(1. + erf((b - m)/s2/s))) *
             (expmx2((-b + m + 2*s*s)/s2/s) *s)) +
             (epsilon*(1. + erf((b - m - 2*s*s)/s2/s))) *
             (expmx2((b - m)/s2/s) *s) +
             2.*(1. + erf((b - m)/s2/s))*
             (1. + erf((b - m - 2*s*s)/s2/s)))) /
             (1. + erf((b - m)/s2/s)) /
             (1. + erf((b - m)/s2/s));

    else {
        fprintf(stderr, "Can't do derivative of %d th moment.\n", r);
        return -1.;
    }
}

double momentDerivSigma(double m, double s, double b, int r)
{
    if (r==1)
        return
            (exp(m + s*s/2.)*(-((epsilon*(b - m + s*s)*
            (1. + erf((b - m)/s2/s))) *
            expmx2((-b + m + s*s)/s2/s)) +
            ((b - m)*epsilon*
            (1. - erf((-b + m + s*s)/s2/s))) *
            expmx2((b - m)/s2) -
            s*s*s*(1. + erf((b - m)/s2/s))*
            (-1. + erf((-b + m + s*s)/s2/s))))/
            (s*s* (1. + erf((b - m)/s2/s)) * (1. + erf((b - m)/s2/s)) );

    else if (r==2)
        return
            (exp(2*(m + s*s))*(-((epsilon*(b - m + 2*s*s)*
            (1. + erf((b - m)/s2/s))) *
            expmx2((-b + m + 2*s*s)/s2/s)) +
            ((b - m)*epsilon*
            (1. + erf((b - m - 2*s*s)/s2/s))) *
            expmx2((b - m)/s2/s) +
            4.*s*s*s*(1. + erf((b - m)/s2/s))*
            (1. + erf((b - m - 2*s*s)/s2/s))))/
            (s*s* (1. + erf((b - m)/(s2*s))) * (1. + erf((b - m)/(s2*s))) );

    else {
        fprintf(stderr, "Can't do derivative of %d th moment.\n", r);
        return -1.0;
    }
}
#endif
#endif

//======================================================================
// Routine: Given an upper limit and a desired mean, sigma, and upper
// limit, solve for the parameters of a log-normal.
// Overall structure: solveLogNormal is a function which solves
// for log_mean and log_sigma, given lin_mean, lin_sigma and lin_up_limit.
// It uses the findZero routine to do the numeric solving. findZero
// is given the function targetFunc to zero.  The parameters are
// communicated between solveLogNormal and targerFunc via these
// three static variables:
static double desired_mean, 
              desired_sigma, 
              desired_up_limit;
#if defined(REGRESSION_TEST) && REGRESSION_TEST==40
static int n_iter;
#endif

// User function for the findZero function:
static void targetFunc(int, double const x[], double f[], double* j[])
{
    // The current logarithmic parameters are passed in the argument
    // array:
    double const& log_mean = x[0];
    double const& log_sigma = x[1];
    double log_up_limit = log(desired_up_limit);

    // The first target function is the deviation from the desired mean:
    double obs_mean = momentLogNormal(log_mean, log_sigma, log_up_limit, 1);
    f[0] = obs_mean - desired_mean;

    // The second target function is the deviation between observed
    // and desired variance:
    double obs_2nd = momentLogNormal(log_mean,log_sigma,log_up_limit,2);
    f[1] = obs_2nd - obs_mean * obs_mean - desired_sigma*desired_sigma;

    #if defined(REGRESSION_TEST) && REGRESSION_TEST==40
    ++n_iter;
    printf("\nIteration %d:\n", n_iter);
    printf("Desired mean=%f sigma=%f, up_limit=%f.\n", 
	   desired_mean, desired_sigma, desired_up_limit);
    printf("Current point: log_mean=%f log_sigma=%f.\n",
	   log_mean, log_sigma);
    printf("Observed mean=%f sigma=%f.\n",
	    obs_mean, sqrt(obs_2nd - obs_mean*obs_mean));
    printf("Eval (%.5f,%.5f) -> (%.1f,%.1f)\n",
	   log_mean, log_sigma,
	   f[0], f[1]);
    #endif

    // The analytic derivatives are numerically very unstable.  Therefore
    // we calculate the derivatives numerically.  If the unit tester
    // is enabled, we also compare them to the analytic derivatives.
    #if defined(REGRESSION_TEST) && REGRESSION_TEST==40
    double analytic_j[2][2];
    analytic_j[0][0] = momentDerivMean(log_mean,log_sigma,log_up_limit,1);
    analytic_j[0][1] = momentDerivSigma(log_mean,log_sigma,log_up_limit,1);

    analytic_j[1][0] = momentDerivMean(log_mean,log_sigma,log_up_limit,2)
                       - 2. * obs_mean
                         * momentDerivMean(log_mean,log_sigma,log_up_limit,1);
    analytic_j[1][1] = momentDerivSigma(log_mean,log_sigma,log_up_limit,2)
                       - 2. * obs_mean
                         * momentDerivSigma(log_mean,log_sigma,log_up_limit,1);
    #endif
    
    // Calculate the derivatives numerically:
    const double step = 1.e-3;
    double x_step[2];
    for (int i=0; i<2; ++i)
        if (fabs(x[i]) < 1.)
            x_step[i] = x[i] + step;
        else
            x_step[i] = x[i] + step * fabs(x[i]);

    double f_step[2][2];
    for (int i_x=0; i_x<2; ++i_x) {
        double const& log_mean_step  = (i_x==0 ? x_step[0] : x[0]);
        double const& log_sigma_step = (i_x==1 ? x_step[1] : x[1]);

        double obs_mean_step = 
            momentLogNormal(log_mean_step,log_sigma_step,log_up_limit,1);
        f_step[0][i_x] = obs_mean_step - desired_mean;
        double obs_2nd_step = 
            momentLogNormal(log_mean_step,log_sigma_step,log_up_limit,2);
        f_step[1][i_x] = obs_2nd_step - obs_mean_step*obs_mean_step
                         - desired_sigma*desired_sigma;
    }
    #if defined(REGRESSION_TEST) && REGRESSION_TEST==40
    printf("Step (%.5f,%.5f) -> (%.1f,%.1f)_0 (%.1f,%.1f)_1\n",
	       x_step[0], x_step[1],
	       f_step[0][0], f_step[1][0],
	       f_step[0][1], f_step[1][1]);
    #endif

    for (int i_f=0; i_f<2; ++i_f)
        for (int i_x=0; i_x<2; ++i_x)
            j[i_f][i_x] = (f_step[i_f][i_x] - f[i_f]) / 
                          (x_step[i_x] - x[i_x]);

    #if defined(REGRESSION_TEST) && REGRESSION_TEST==40
    printf("Deriv anal: (%f,%f)_0 (%f,%f)_1\n",
	   analytic_j[0][0], analytic_j[1][0], 
	   analytic_j[0][1], analytic_j[1][1]);
    printf("Deriv numx: (%f,%f)_0 (%f,%f)_1\n",
	   j[0][0], j[1][0], j[0][1], j[1][1]);

    bool deriv_ok = true;
    for (int i_f=0; i_f<2; ++i_f)
        for (int i_x=0; deriv_ok && i_x<2; ++i_x) {
            double problem = (analytic_j[i_f][i_x] - j[i_f][i_x]) /
                             (analytic_j[i_f][i_x] + j[i_f][i_x]);
            if (fabs(problem) > 0.1) {
                deriv_ok = false;
                printf("At i=%d mean=%.5f sigma=%.5f: "
                       "Deriv df%d/dx%d wrong.\n"
                       "    Analytic %f, numeric %f.\n",
                       n_iter, log_mean, log_sigma, i_f, i_x, 
                       analytic_j[i_f][i_x], j[i_f][i_x]);
            }
        }

    #endif
}

// Given the desired linear parameters of a log normal, calculate the
// actual logarithmic parameters.  Return true if it succeeded,
// false otherwise.
bool solveLogNormal(double lin_mean,     // Input
                    double lin_sigma, 
                    double lin_up_limit,
                    double& log_mean,    // Output
                    double& log_sigma,
                    double& log_up_limit)
{
    desired_mean     = lin_mean;
    desired_sigma    = lin_sigma;
    desired_up_limit = lin_up_limit;
    log_up_limit = log(lin_up_limit);

    // Educated guess of starting point:
    double x[2];
    x[1] = sqrt(log(lin_sigma*lin_sigma / 
                    lin_mean/lin_mean +1.));
    x[0] = log(lin_mean) - x[1]*x[1]/2.;

    #if defined(REGRESSION_TEST) && REGRESSION_TEST==40
    printf("Starting zero finder at log mu=%.5f sigma=%.5f\n", x[0], x[1]);
    printf("Corresponds to mean=%f sigma=%f.\n",
	   momentLogNormal(x[0], x[1], log_up_limit, 1),
	   sqrt(momentLogNormal(x[0], x[1], log_up_limit, 2) -
		momentLogNormal(x[0], x[1], log_up_limit, 1) *
		momentLogNormal(x[0], x[1], log_up_limit, 1)));
    #endif
 
    // Solve it:
    bool finder_success = findZero(2, targetFunc, x, 500, 0.000001, 0.001);

    // Copy the found parameters to the output:
    if (finder_success) {
        log_mean = x[0];
        log_sigma = x[1];
    }
  
    return finder_success;
}

//======================================================================
// Generate a normal-distributed number, smaller than the upper limit,
// using the box-muller algorithm, as described in the numerical 
// recipes book:
static double normal_random_limit(double up_limit)
{
    double nr = 0.0; // This is the normal random number we want
    bool got_one = false;
    while (!got_one) {
        double v1 = 2. * drand48() - 1.;
        double v2 = 2. * drand48() - 1.;
        double vr = v1*v1 + v2*v2;

        if (vr > 0.0 || vr <= 1.0) {
            nr = v1 * sqrt(-2. * log(vr)/vr);
            if (nr <= up_limit)
                got_one = true;
            if (!got_one) {
                nr = v2 * sqrt(-2. * log(vr)/vr);
                if (nr <= up_limit)
                    got_one = true;
            }
        }
    }
    return nr;
}

// ======================================================================
// Main program.
#if !defined(REGRESSION_TEST) || \
    (defined(REGRESSION_TEST) && REGRESSION_TEST==50)
void usage(char *const prog_name, 
           const std::string& message, 
	   const std::string& details = "")
{
    if (details != "") {
	fprintf(stderr, "%s: %s\n", message.c_str(), details.c_str());
    }
    else {
	fprintf(stderr, "%s\n", message.c_str());
    }

    fprintf(stderr,
            "Usage: %s [-v] mean std_dev up_limit [rand_int]\n"
            "Generates a random number, distributed with a "
            "truncated log-normal distribution.\n"
            "You specify desired mean, std. deviation, upper limit, "
            "and a random integer.\n"
            "The random number is printed on stdout.  "
            "If the parameters make no sense, an error message is "
            "printed on stderr.\n"
            "Option -v also outputs the parameters of the distribution on "
            "stderr.\n",
            prog_name);

    exit(1);
}

int main(int argc, char* argv[])
{
    int iarg = 1;

    // Look for the -v argument:
    bool verbose_params = false;
    if (strcmp(argv[iarg], "-v")==0) {
        verbose_params = true;
        ++iarg;
    }

    if (argc-iarg < 3 || argc-iarg > 4)
        usage(argv[0], "Error: Wrong number of arguments!");

    // Decode the arguments from the command line:
    char* used_argument;
    double lin_mean = strtod(argv[iarg], &used_argument);
    if (used_argument == argv[iarg])
        usage(argv[0], "Error: First argument is not a number", argv[iarg]);
    if (lin_mean <= 0.0)
        usage(argv[0], "Error: Mean must be strictly positive!");
    ++iarg;

    double lin_sigma = strtod(argv[iarg], &used_argument);
    if (used_argument == argv[iarg])
        usage(argv[0], "Error: Second argument is not a number", argv[iarg]);
    if (lin_sigma <= 0.0)
        usage(argv[0], "Error: Standard deviation must be strictly positive!");
    ++iarg;

    double lin_up_limit = strtod(argv[iarg], &used_argument);
    if (used_argument == argv[iarg])
        usage(argv[0], "Error: Third argument is not a number", argv[iarg]);
    if (lin_up_limit <= 0.0)
        usage(argv[0], "Error: Upper limit must be strictly positive!");
    ++iarg;

    long random_seed = 0;
    bool is_random_seed_set = false;
    if (iarg<argc) {
        is_random_seed_set = true;
        random_seed = strtol(argv[iarg], &used_argument, 0);
        if (used_argument == argv[iarg])
            usage(argv[0], "Error: Fourth argument is not a number",
                  argv[iarg]);
        ++iarg;
    }

    if (verbose_params)
        fprintf(stderr, 
                "Input linear parameters: Mean %f, stddev %f limit %f\n",
                lin_mean, lin_sigma, lin_up_limit);

    // Try to solve for the log-normal:
    double log_mean, log_sigma, log_up_limit;
    bool solution_exists = solveLogNormal(lin_mean, lin_sigma, lin_up_limit,
                                          log_mean, log_sigma, log_up_limit);

    if (!solution_exists) {
        fprintf(stderr, 
                "No log-normal solution exists for parameters (%f+-%f)<%f\n",
                lin_mean, lin_sigma, lin_up_limit);
        exit(2);
    }

    if (verbose_params)
        fprintf(stderr, 
                "Solved log parameters: Mean %f, stddev %f limit %f\n",
                log_mean, log_sigma, log_up_limit);

    // If the user wants no random numbers, we're done:
    if (!is_random_seed_set)
        return 0;

    // Initialize our own random number generator:
    srand48(random_seed);

    double normal_up_limit = ( log_up_limit - log_mean ) / log_sigma;

    #if !defined(REGRESSION_TEST)
    // Normal operations: Make one random number, scale it,
    // return it:
    double nr = normal_random_limit(normal_up_limit);
    double r = exp(log_sigma * nr + log_mean);
    printf("%f\n", r);

    #else /* REGRESSION_TEST */
    // Make a histogram of 1 million random numbers
    const int n_hist = 1000;
    const int n_samples = 1000000;
    double hist_limit = lin_up_limit;
    double hist_step  = hist_limit / n_hist;
    int* hist = new int[n_hist +1];
    for (int i=0; i<n_hist+1; ++i)
        hist[i] = 0;
    fprintf(stderr, 
            "Please wait, generating histogram with %d entries ...\n", 
            n_samples);

    for (int i=0; i<n_samples; ++i) {
        double nr = normal_random_limit(normal_up_limit);
        double r = exp(log_sigma * nr + log_mean);

        int i_hist = (int) (r / hist_step );
        if (i_hist >= 0 && i_hist < n_hist+1)
            ++hist[i_hist];
        else
            fprintf(stderr, 
                    "Warning: Random number %f generated out of bounds.\n",
                    r);
    }

    // The funny division of the 2nd argument is to make sure the integral
    // over the histogram is ==1:
    for (int i=0; i<n_hist+1; ++i)
        printf("%f %f\n", i*hist_step, 
              (double) hist[i] / (double) n_samples / hist_step);
    #endif
}
#endif

// ========================================================================
// Regression tester: Numeric integration of the moments.
#if defined(REGRESSION_TEST) && REGRESSION_TEST==10
int main(int argc, char* argv[])
{
    if (argc != 4) {
        printf("Regression tester for moments of the truncated log-normal "
               "distribution.\n");
        printf("Usage: %s mean sigma limit.\n", argv[0]);
        printf("Mean, sigma and limit are on the linear scale.\n");
        exit(1);
    }

    double lin_mean    = atof(argv[1]);
    double lin_sigma   = atof(argv[2]);
    double lin_uplimit = atof(argv[3]);

    // The logarithmic sigma is chosen such that the distribution without
    // upper limit has the correct mean and sigma:
    double log_sigma   = sqrt(log(lin_sigma*lin_sigma / 
                                  lin_mean/lin_mean +1.));
    double log_mean    = log(lin_mean) - log_sigma*log_sigma/2.;
    double log_uplimit = log(lin_uplimit);

    printf("Linear parameters: mean %f\n"
           "                   std_dev %f\n"
           "                   upper limit %f\n",
           lin_mean, lin_sigma, lin_uplimit);
    printf("Log parameters:    mean %f\n"
           "                   std_dev %f\n"
           "                   upper limit %f\n",
           log_mean, log_sigma, log_uplimit);

    printf("\nExpected (from Arif's formula):\n"
           "Mean: %f, std dev: %f\n", 
           momentLogNormal(log_mean, log_sigma, log_uplimit, 1),
           sqrt(momentLogNormal(log_mean, log_sigma, log_uplimit, 2) -
                momentLogNormal(log_mean, log_sigma, log_uplimit, 1)*
                momentLogNormal(log_mean, log_sigma, log_uplimit, 1)));

    // Now numerically integrate the log-normal.  For accuracy, we
    // integrate from the bottom up to the mean, and from the top
    // down to the mean separately (so we don't add smaller numbers
    // to bigger ones).
    // Integrate in 10000 steps from 0 to mean,
    //           in 10000 steps from mean to mean + sigma
    //           in  9000 steps from mean+sigma to mean+10*sigma
    //           in  9000 steps from mean+10*sigma to mean+100*sigma
    //           in  9000 steps from mean+100*sigma to mean+1000*sigma
    printf("\nNumeric integration:\n");
    double s_0 = 0.0; // The totals for the three moments, lower half
    double s_1 = 0.0;
    double s_2 = 0.0;
    double t_0 = 0.0; // The totals for the three moments, upper half
    double t_1 = 0.0;
    double t_2 = 0.0;
    int n_integral = 0; // Count how many samples we took

    // Integrate from 0 to the mean
    double step = lin_mean / 10000.;
    for (double xl=0.0; 
         xl<lin_mean && xl<lin_uplimit; 
         xl+=step) {
        double d = logNormal(xl + step/2., log_mean, log_sigma, lin_uplimit);
        s_0 += step * d;
        s_1 += step * xl * d;
        s_2 += step * xl * xl * d;
        ++n_integral;
    }

    // Integrate from mean+1000*sigma down to mean+100*lin_sigma
    if (lin_mean + 100.*lin_sigma < lin_uplimit) {
        step = lin_sigma / 10.;
        double upper_end = lin_mean + 1000. * lin_sigma;
        if (upper_end > lin_uplimit)
            upper_end = lin_uplimit;
        for (double xu=upper_end; 
             xu > lin_mean + 100.*lin_sigma;
             xu -= step) {
            double d = logNormal(xu+step/2., log_mean, log_sigma, lin_uplimit);
            t_0 += step * d;
            t_1 += step * xu * d;
            t_2 += step * xu * xu * d;
            ++n_integral;
        }
    }

    // Integrate from mean+100*sigma down to mean+10*sigma
    if (lin_mean + 10.*lin_sigma < lin_uplimit) {
        step = lin_sigma / 100.;
        double upper_end = lin_mean + 100. * lin_sigma;
        if (upper_end > lin_uplimit)
            upper_end = lin_uplimit;
        for (double xu=upper_end; 
             xu >= lin_mean + 10.*lin_sigma;
             xu -= step) {
            double d = logNormal(xu+step/2., log_mean, log_sigma, lin_uplimit);
            t_0 += step * d;
            t_1 += step * xu * d;
            t_2 += step * xu * xu * d;
            ++n_integral;
        }
    }

    // Integrate from mean+10*sigma down to mean+sigma
    if (lin_mean + lin_sigma < lin_uplimit) {
        step = lin_sigma / 1000.;
        double upper_end = lin_mean + 10. * lin_sigma;
        if (upper_end > lin_uplimit)
            upper_end = lin_uplimit;
        for (double xu=upper_end;
             xu >= lin_mean + lin_sigma;
             xu -= step) {
            double d = logNormal(xu+step/2., log_mean, log_sigma, lin_uplimit);
            t_0 += step * d;
            t_1 += step * xu * d;
            t_2 += step * xu * xu * d;
            ++n_integral;
        }
    }

    // Integrate from mean+sigma down to mean
    if (lin_mean < lin_uplimit) {
        step = lin_sigma / 10000.;
        double upper_end = lin_mean + lin_sigma;
        if (upper_end > lin_uplimit)
            upper_end = lin_uplimit;
        for (double xu=upper_end;
             xu >= lin_mean;
             xu -= step) {
            double d = logNormal(xu+step/2., log_mean, log_sigma, lin_uplimit);
            t_0 += step * d;
            t_1 += step * xu * d;
            t_2 += step * xu * xu * d;
            ++n_integral;
        }
    }

    s_0 += t_0;
    s_1 += t_1;
    s_2 += t_2;

    printf("Integrated over %d samples.\n", n_integral);
    printf("Numeric integration: total integral: %f\n"
           "Mean: %f, std dev: %f\n",
           s_0, s_1/s_0, sqrt(s_2/s_0 - s_1*s_1/s_0/s_0));

}
#endif /* REGRESSION_TEST==10 */

// ========================================================================
// Regression tester: Compare analytic and numeric derivatives of the
// moments.
#if defined(REGRESSION_TEST) && REGRESSION_TEST==20
// ======================================================================
int main(int argc, char* argv[])
{
    if (argc != 4) {
        printf("Regression test for derivatives of moments of log-normal\n");
        printf("Usage: %s mean sigma limit.\n", argv[0]);
        printf("Mean, sigma and limit are on the linear scale.\n");
        exit(1);
    }

    double lin_mean    = atof(argv[1]);
    double lin_sigma   = atof(argv[2]);
    double lin_uplimit = atof(argv[3]);

    double log_mean    = log(lin_mean);
    double log_sigma   = log((lin_mean+lin_sigma)/lin_mean);
    double log_uplimit = log(lin_uplimit);

    printf("Linear parameters: mean %f\n"
           "                   std_dev %f\n"
           "                   upper limit %f\n",
           lin_mean, lin_sigma, lin_uplimit);
    printf("Log parameters:    mean %f\n"
           "                   std_dev %f\n"
           "                   upper limit %f\n",
           log_mean, log_sigma, log_uplimit);

    // Test at 3 points each in mean, sigma, and upper limit space:
    double t_mean[3];
    t_mean[0] = log_mean -log(5.);
    t_mean[1] = log_mean;
    t_mean[2] = log_mean +log(5.);
    double t_sigma[3];
    t_sigma[0] = log_sigma /2.;
    if (t_sigma[0] < 0.001) t_sigma[0] = 0.001;
    t_sigma[1] = log_sigma;
    t_sigma[2] = log_sigma *2.;
    double t_uplimit[3];
    t_uplimit[0] = log_uplimit -log(10.);
    t_uplimit[1] = log_uplimit;
    t_uplimit[2] = log_uplimit +log(10.);

    double delta_m = log_mean / 100.;
    double delta_s = log_sigma / 100.;

    printf("Mean  StdDv Limit R  Moment       "
           "dMom/dm(a)   dMom/dm(n)    dMom/ds(a)   dMom/ds(n)\n");
    char const* fmt[3] = {
        "%5.3f %5.3f %5.2f %d  %12.7f  %12.7f:%12.7f %12.7f:%12.7f\n",
        "%5.3f %5.3f %5.2f %d  %12.4f  %12.4f:%12.4f %12.4f:%12.4f\n",
        "%5.3f %5.3f %5.2f %d  %12.1f  %12.1f:%12.1f %12.1f:%12.1f\n" };
    for (int r=1; r<3; ++r) {
        for (int i=0; i<3; ++i) {
            double m = t_mean[i];
            for (int j=0; j<3; ++j) {
                double s = t_sigma[j];
                for (int k=0; k<3; ++k) {
                    double b = t_uplimit[k];

                    double f = momentLogNormal(m,s,b,r);
                    double f_m = ( momentLogNormal(m+delta_m,s,b,r) -
                                   momentLogNormal(m,s,b,r) ) /
                                 delta_m;
                    double f_s = ( momentLogNormal(m,s+delta_s,b,r) -
                                   momentLogNormal(m,s,b,r) ) /
                                 delta_s;
                    
                    printf(fmt[r],
                           m, s, b, r, f,
                           momentDerivMean(m,s,b,r), f_m,
                           momentDerivSigma(m,s,b,r), f_s);
                }
            }
        }
    }
}
#endif /* REGRESSION_TEST==20 */

// ========================================================================
// Regression tester: Solve a simple linear 2-dimensional equation system.
#if defined(REGRESSION_TEST) && REGRESSION_TEST==30

int main(int, char*[])
{
    double** good = new double*[2]; // Should be solvable
    good[0] = new double[2];
    good[1] = new double[2];
    good[0][0] = 3.0; good[0][1] = 4.0; 
    good[1][0] = 1.0; good[1][1] = 2.0;

    double** bad = new double*[2]; // Not solvable, not linearly independent.
    bad[0] = new double[2];
    bad[1] = new double[2];
    bad[0][0] = 3.0; bad[0][1] = 6.0; 
    bad[1][0] = 1.0; bad[1][1] = 2.0;

    double* rhs = new double[2];
    rhs[0] = 1.0;
    rhs[1] = 1.0;

    double determinant;
    double x[2];

    bool worked = solve2(good, rhs, determinant, x);
    printf("First test:\n"
           "Worked=%d, det=%f (should be true,2)\n", worked, determinant);
    printf("Result: (%f,%f) (should be (-1,1))\n", x[0], x[1]);
    
    worked = solve2(bad, rhs, determinant, x);
    printf("\n"
           "Second test:\n"
           "Worked=%d, det=%f (should be false,0)\n", worked, determinant);
}
#endif /* REGRESSION_TEST==30 */

// ========================================================================
// Regression tester: Solve some fun 2-dimensional non-linear equations.
#if defined(REGRESSION_TEST) && REGRESSION_TEST >= 31 && REGRESSION_TEST <= 39

// The thing we are trying to solve is
// - Test 31: x + y^2 = 10
//            x + y^3 = 20
// - Test 32: log x + 10 * sin(y^2) = 10
//            e^x   + y^3 = 20
void testFunc(int, double const x[], double f[], double* j[])
{
    #if REGRESSION_TEST==31
    f[0] = x[0] + x[1]*x[1] -10.0;
    f[1] = x[0] + x[1]*x[1]*x[1] - 20.0;
    j[0][0] = 1.;
    j[0][1] = 2. * x[1];
    j[1][0] = 1.;
    j[1][1] = 3. * x[1]*x[1];

    #else
    f[0] = log(x[0]) + 10. * sin(x[1]*x[1]) -10.0;
    f[1] = exp(x[0]) + x[1]*x[1]*x[1] - 20.0;
    j[0][0] = 1. / x[0];
    j[0][1] = 10. * cos(x[1]*x[1]) * 2.*x[1];
    j[1][0] = exp(x[0]);
    j[1][1] = 3. * x[1]*x[1];
    #endif

    printf("Eval (%5.2f,%5.2f) -> (%5.2f,%5.2f) and "
           "((%5.2f,%5.2f)_0,(%5.2f,%5.2f)_1)\n",
           x[0], x[1], f[0], f[1], j[0][0], j[0][1], j[1][0], j[1][1]);
}

int main(int, char*[])
{
    double x[2];
    x[0] = 3.0;
    x[1] = 2.0;

    printf("Starting zero finder at (%5.2f,%5.2f)\n", x[0], x[1]);
    bool success = findZero(2, testFunc, x, 50, 0.001, 0.001);
    printf("Zero finder %s succeed.\n", (success ? "DID" : "DID NOT"));
    printf("Zero finder ended at at (%5.2f,%5.2f)\n", x[0], x[1]);
    
    return 0;
}
#endif /* REGRESSION_TEST==31...39 */

//======================================================================
// Unit tester for solveLogNormal:
#if defined(REGRESSION_TEST) && REGRESSION_TEST==40

int main(int argc, char* argv[])
{
    if (argc != 4) {
        printf("Usage: %s mean sigma limit.\n", argv[0]);
        printf("Mean, sigma and limit are on the linear scale.\n");
        exit(1);
    }

    desired_mean        = atof(argv[1]);
    desired_sigma       = atof(argv[2]);
    double lin_up_limit = atof(argv[3]);

    double log_mean, log_sigma, log_up_limit;
    bool success = solveLogNormal(desired_mean, desired_sigma, lin_up_limit,
                                  log_mean, log_sigma, log_up_limit);

    printf("Zero finder %s succeed.\n", (success ? "DID" : "DID NOT"));
    printf("Needed %d iterations.\n", n_iter);

    if (success) {
        printf("Zero finder ended at log mu=%.5f sigma=%.5f\n", 
               log_mean, log_sigma);

        printf("Corresponds to mean=%f sigma=%f.\n",
               momentLogNormal(log_mean,log_sigma,log_up_limit,1),
               sqrt(momentLogNormal(log_mean,log_sigma,log_up_limit,2) -
                    momentLogNormal(log_mean,log_sigma,log_up_limit,1) *
                    momentLogNormal(log_mean,log_sigma,log_up_limit,1)));
    }
    return 0;
}
#endif


