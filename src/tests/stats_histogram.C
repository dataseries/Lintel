#include <StatsHistogram.H>
#include <StatsRW.H>
#include <StatsMaker.H>
#include <MersenneTwisterRandom.H>
#include "streamcompat.H"

int
main()
{
    MersenneTwisterRandom rand(1776); // An excellent year.

    std::cout << "#>Rome-1-9\n";
    Stats a;

    int i;

    for(i=0;i<1000;i++) {
	a.add(rand.randDouble());
    }

    std::cout << "stats simple {\n";
    a.printRome(2,std::cout);
    std::cout << "};\n\n";

    StatsHistogramUniform b(20,0.0,1.0);

    for(i=0;i<1000;i++) {
	b.add(rand.randDouble());
    }

    std::cout << "stats histogram_uniform {\n";
    b.printRome(2,std::cout);
    std::cout << "};\n\n";

    StatsHistogramLog c(20,1.0,exp(1.0));

    for(i=0;i<1000;i++) {
	c.add(exp(rand.randDouble()));
    }

    std::cout << "stats histogram_log {\n";
    c.printRome(2,std::cout);
    std::cout << "};\n\n";

    StatsHistogramLogAccum d(20,1.0,exp(1.0));

    for(i=0;i<1000;i++) {
	d.add(exp(rand.randDouble()));
    }

    std::cout << "stats histogram_log_accum {\n";
    d.printRome(2,std::cout);
    std::cout << "};\n\n";

    std::vector<double> ranges(6);
    for(i=0;i<6;i++) {
	ranges[i] = 0.2 * (double)i;
    }
    StatsHistogramGroup e(StatsHistogram::Uniform,4, ranges);

    for(i=0;i<1000;i++) {
	e.add(rand.randDouble());
    }

    std::cout << "stats histogram_group {\n";
    e.printRome(2,std::cout);
    std::cout << "};\n\n";

    StatsRW f(&a,&b,&c);
    for(i=0;i<1000;i++) {
	f.add(rand.randDouble(),StatsRW::Read);
    }
    for(i=0;i<1000;i++) {
	f.add(rand.randDouble(),StatsRW::Write);
    }
    std::cout << "stats RW {\n";
    f.printRome(2,std::cout);
    std::cout << "};\n\n";

    StatsMaker::setModeHistogram("default",StatsHistogram::Uniform,4,0.0,1.0);
  
    Stats *g = StatsMaker::make("test");
    for(i=0;i<1000;i++) {
	g->add(rand.randDouble());
    }
    std::cout << "stats maker_histogram {\n";
    g->printRome(2,std::cout);
    std::cout << "};\n\n";

    // check scaling behavior
    StatsHistogramLog h(4, 1, 16, true);
    h.add(1.2);
    h.add(3.4);
    h.add(4.1);
    h.add(14);
    h.add(19.4);
    h.add(255.9);
    h.add(0.8);
    h.add(0.004);

    std::cout << "stats histogram_log_scalable {\n";
    // NB: bin counts should be 1,1,4,2, high = 256, low = 1/256
    h.printRome(2,std::cout);
    std::cout << "};\n\n";

    // check growing behavior
    StatsHistogramUniform j(4, 10, 14, false, true);
    j.add(11.2);
    j.add(14.0);
    j.add(15.1);
    j.add(15.2);
    j.add(17.0);
    j.add(2.9);

    std::cout << "stats histogram_uniform_scalable {\n";
    // NB: bin counts should be ???1,1,4,2, high = 18, low = 1
    j.printRome(2,std::cout);
    std::cout << "};\n\n";


    exit(0);
}
