/* -*-C++-*- */
/*
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Class for random-number generation
*/


#ifndef _RANDOM_MWM_H_
#define _RANDOM_MWM_H_


#include "Random.H"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>

#define MWM_BETACONST 1.3862944
#define MWM_MAX_TRIES 50

class RandMwm : public Rand {
private:
    RandSeed seed;
protected:
    double mu_c; //mean of coarse scale coeffs     
    double std_c; //their std
    std::vector<double> p; //beta params at each scale
    long state; // MWM random numbers are generated in batches of
		// 2^n where n=p.size(). The state variable is
                // used to indicate if a fresh batch of MWM random
                // variables is to be drawn
    long N_s;   // p.size()
    long batchlength; // 2^N_s

    std::vector<double> gen_sig; //Generated random variables (1 batch)

	void randn (std::vector <double>& r, long l=1, double sigma=1, double mu=0);
	void betarnd (std::vector <double>& r, double p,double q,long l=1);
	void gen_beta_mwm();

public:
    RandMwm(double m, double s, std::vector<double>& bparams)
	: Rand(), seed(), mu_c(m), std_c(s), p(bparams), state(0)  
	{N_s=p.size(); batchlength=2^N_s;};
    RandMwm(double m, double sd, std::vector<double>& bparams, RandSeeder* s)
	: Rand(), seed(s), mu_c(m), std_c(sd), p(bparams), state(0)  {};
    virtual ~RandMwm()
	{N_s=p.size(); batchlength=2^N_s;};

    virtual double draw();
    virtual double mean() { return  mu_c; };

};

#endif
