/* -*-C++-*-
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    calculate stats on data
*/

/*
   Calculates count, min, max, mean, stddev, 19% confidence interval,
   variance, and relconf95(?) of a bunch of data. The data is assumed to
   be in a data file (single column only), whose name is taken in on the
   command line. All data items are treated as doubles.
*/

#include <stdlib.h>
#include <LintelAssert.H>
#include <Stats.H>

int
main(int argc, char *argv[])
{
    if (argc != 2) {
	fprintf(stderr,"Usage: %s <data-file>\n",argv[0]);
	exit(1);
    }

    FILE *data_file = fopen(argv[1],"r");
    AssertAlways(data_file != NULL,("Can't open %s\n",argv[1]));

    Stats data_values;
    
    while(!feof(data_file)) {
	double data = -1.0;
	fscanf(data_file,"%lf", &data);
	if (data!=-1.0)
	    data_values.add(data);
    }
    printf("Summary: count %ld min %.5g max %.5g mean %.5g stddev %.5g conf95 %.5g variance %.5g relconf95 %.5g\n",
	   data_values.count(),
	   data_values.min(),
	   data_values.max(),
	   data_values.mean(),
	   data_values.stddev(),
	   data_values.conf95(),
	   data_values.variance(),
	   data_values.relconf95());

    return 0;
}

    
