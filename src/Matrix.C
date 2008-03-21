/* -*-C++-*-
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Matrix functions
*/

#include <stdarg.h>

#include <Lintel/Matrix.hpp>

Matrix::Matrix(unsigned int rows, unsigned int cols)
{
    resize(rows,cols);
}

Matrix::~Matrix()
{
    delete [] data;
}

void
Matrix::resize(unsigned int rows, unsigned int cols)
{
    nrows = rows;
    ncols = cols;
    data = new double[rows * cols];
    setConstant(0.0);
}

void
Matrix::setRow(unsigned int row, ...)
{
    va_list ap;
    va_start(ap,row);

    for(unsigned int i=0;i<ncols;i++) {
	set(row,i,va_arg(ap, double));
    }
    Assert(1,isnan(va_arg(ap,double)));
    va_end(ap);
}

inline void
Matrix::setConstant(double value)
{
    for(unsigned int i = 0;i<nrows * ncols; i++) {
	data[i] = value;
    }
}

inline void 
Matrix::swapRows(unsigned int row1, unsigned int row2)
{
    Assert(1,row1 < nrows && row2 < nrows);
    for(unsigned int i=0;i<ncols;i++) {
	double tmp = get(row1,i);
	set(row1,i,get(row2,i));
	set(row2,i,tmp);
    }
}

inline void 
Matrix::divRow(unsigned int row, double val)
{
    Assert(1,row < nrows && Double::abs(val) > 0);
    for(unsigned int i=0;i<ncols;i++) {
	set(row,i,get(row,i)/val);
    }
}

inline void
Matrix::accumMultiply(unsigned int into_row, unsigned int from_row,
		      double value)
{
    Assert(1,into_row < nrows && from_row < nrows);
    for(unsigned int i=0;i<ncols;i++) {
	set(into_row,i,get(into_row,i) + value * get(from_row,i));
    }
}

int
Matrix::gaussEliminate(int maxcolumn)
{
    unsigned int column;
    if (maxcolumn == -1) {
	maxcolumn = std::min(nrows,ncols);
    }
    int eliminated_cols = 0;
    for(column=0;(int)column<maxcolumn;column++) {
	if (eliminated_cols == (int)nrows)
	    break; // can't eliminate more than this many cols.
	unsigned int maxrow = eliminated_cols;
	for(unsigned int i=maxrow+1;i<nrows;i++) {
	    if (Double::abs(get(maxrow,column)) < Double::abs(get(i,column))) {
		maxrow = i;
	    }
	}
	swapRows(maxrow,eliminated_cols);
	if (Double::eq(get(eliminated_cols,column),0)) {
	    continue;
	} 
	divRow(eliminated_cols,get(eliminated_cols,column));
	for(unsigned int i=eliminated_cols + 1;i<nrows;i++) {
	    accumMultiply(i,eliminated_cols,-get(i,column));
	}
	++eliminated_cols;
    }
    if (eliminated_cols == maxcolumn) {
	// Now have a lower triangular matrix; back substitute.
	for(--column;column>0;--column) {
	    for(unsigned int i=0;i<column;i++) {
		accumMultiply(i,column,-get(i,column));
	    }
	}
    }
		
    return eliminated_cols;
}

void
Matrix::print()
{
    for(unsigned int i=0;i<nrows;i++) {
	for(unsigned int j=0;j<ncols;j++) {
	    printf("%.6g ",get(i,j));
	}
	printf("\n");
    }
}

void
Matrix::selfTest()
{
    AssertAlways(isnan(Double::NaN),("isnan failed\n"));
    Matrix test1(2,3);
    
    test1.set(0,0,2);
    test1.set(0,2,5);
    test1.set(1,1,2);
    test1.set(1,2,10);
    
    test1.gaussEliminate(2);
    AssertAlways(Double::eq(test1.get(0,0),1) && 
		 Double::eq(test1.get(0,1),0) && 
		 Double::eq(test1.get(0,2),2.5) &&
		 Double::eq(test1.get(1,0),0) &&
		 Double::eq(test1.get(1,1),1) && 
		 Double::eq(test1.get(1,2),5),("selfTest() failed\n"));
    
    test1.setRow(0,25.0,4.0,12.0,Double::NaN);
    test1.setRow(1,13.0,7.0,11.0,Double::NaN);
    //    test1.print();
    test1.gaussEliminate(2);
    double a = test1.get(0,2);
    double b = test1.get(1,2);
    AssertAlways(Double::eq(25*a+4*b,12) &&
		 Double::eq(13*a+7*b,11),("selfTest() failed\n"));

    Matrix test2(4,5);

    test2.setRow(0,1.0,0.0,0.0,1.0,5.0,Double::NaN);
    test2.setRow(1,0.0,1.0,0.0,1.0,5.0,Double::NaN);
    test2.setRow(2,0.0,0.0,1.0,1.0,5.0,Double::NaN);
    test2.setRow(3,1.0,0.0,0.0,1.0,5.0,Double::NaN);
    test2.gaussEliminate();
    //    test2.print();

    Matrix test3(5,5);
    test3.setRow(0,8192.0,1.0,1.0,1.0,40.3349,Double::NaN);
    test3.setRow(1,8192.0,1.0,2.0,1.0,71.1314,Double::NaN);
    test3.setRow(2,4096.0,1.0,1.0,1.0,97.7704,Double::NaN);
    test3.setRow(3,8192.0,2.0,2.0,1.0,101.864,Double::NaN);
    test3.setRow(4,8192.0,2.0,2.0,1.0,101.864,Double::NaN);
    test3.gaussEliminate(4);
    //    test3.print();
    //    printf("%.7g\n",8192*test3.get(0,4) + test3.get(1,4) + test3.get(2,4) + test3.get(3,4));
}

