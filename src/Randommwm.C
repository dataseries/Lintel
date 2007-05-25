/* -*-C++-*-
   (c) Copyright 2002-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Class for random-number generation
*/

#include <time.h>
#include <sys/time.h>

#include <Lintel/LintelAssert.H>
#include <Lintel/Randommwm.H>

void RandMwm::randn (std::vector <double>& r, long l,double sigma, double mu)
{
        r.clear();
        double temp;
        for (long i=1; i<=l;i++) {
                temp = -2.0*log(erand48(seed.seeds));
                r.push_back (mu + sigma * sqrt(temp) * cos(2.0*M_PI*erand48(seed.seeds)));
        }
}

void RandMwm::betarnd (std::vector <double>& r, double p,double q,long l)
{
        r.clear();
        double alp=p+q;
        double B,v,u1,u2,w;
        if ((p<=1)||(q<=1))
                B= ((1/p) > (1/q)) ? 1/p : 1/q;
        else
                B=sqrt ((alp-2)/(2*p*q-alp));

        double G=p+1/B;
        long n=l, i;

        while ((int)r.size()<l)
        {
                for (i=1; i<=n; i++)
                {
                        u1=erand48(seed.seeds); u2=erand48(seed.seeds);
                        v=(B*log(u1/(1-u1)));
                        w=(p*exp(v));
                        if ( (alp*log (alp/(q+w))+G*v-MWM_BETACONST) >= (log(u1*
u1*u2)))
                                r.push_back(w/(q+w));
                }
                n=l-r.size();
        }
}

void RandMwm::gen_beta_mwm()
{
	long tries=0, scale, i, n1;

        //Synthesize coarsest scale signal:
        double c=pow(2.0, (double)N_s/2);
        bool flag=false;
        std::vector <double>::iterator j;
        std::vector <double> tmpsig,a;
	gen_sig.clear();
        while ((tries++<MWM_MAX_TRIES)&&(flag==false))
        {
                randn(gen_sig, 1, std_c/c, mu_c/c);
                flag=true;
                for (j=gen_sig.begin(); j!=gen_sig.end(); j++)
                        if (*j <= 0) {
                                flag=false;
                                break;
                        }
        }

        for (scale=1; scale<=N_s; scale++)
        {
                n1=(long)pow(2.0, (double)scale-1);
                betarnd(a,p[scale-1],p[scale-1],n1);
                tmpsig.clear();
                for (i=1; i<=n1; i++)
                {
                        tmpsig.push_back(gen_sig[i-1]*(2-2*a[i-1]));
                        gen_sig[i-1]=(gen_sig[i-1]*(2*a[i-1]));
                }
                a.clear();
                for (i=1; i<=(int)tmpsig.size(); i++)
                {
                        a.push_back(gen_sig[i-1]);
                        a.push_back(tmpsig[i-1]);
                }
                gen_sig=a;


                /* for (j=tmpsig.begin(); j!=tmpsig.end(); j++)
                        gen_sig.push_back(*j); */
        }
}


double RandMwm::draw()
{
	if ( (state<1)||(state>batchlength) )
	{
		gen_sig.clear();
		gen_beta_mwm();
		state=1;
	}
	
	state++;
	return (gen_sig[state-1]);
}

#if 0
int main ()  // FOR TESTING
{

	//parameters chosen to match the example program that
        //comes with MWM matlab package

	double mu_c, std_c;
	std::vector <double> p;

	// Sample values obtained from Rubicon
	mu_c=0.1210781;
	std_c=0.0145513;

	p.push_back(19.7006);
	p.push_back(23.5817);
	p.push_back(20.1524);
	p.push_back(9.5747);
	p.push_back(6.6421);
	p.push_back(4.9687);
	p.push_back(3.2089);
	p.push_back(2.0937);
	p.push_back(1.7823);
	p.push_back(1.0705);
	p.push_back(0.5742);

	Rand* mwm=new RandMwm(mu_c,std_c,p);
	RandMwm* mwm2=new RandMwm(mu_c,std_c,p);
	long testlength=4096;
	for (unsigned i=0; i<testlength; i++)
	{
		printf("%g\n",mwm->draw());
		printf("%g\n",mwm2->draw());
	}

	return(0);
};
#endif
