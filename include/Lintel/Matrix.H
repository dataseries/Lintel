/* -*-C++-*-
*******************************************************************************
*
* File:         Matrix.H
* RCS:          $Header: /mount/cello/cvs/Lintel/include/Lintel/Matrix.H,v 1.3 2003/07/30 00:17:17 anderse Exp $
* Description:  Various useful matrix manipulation functions
* Author:       Eric Anderson
* Created:      Mon Jun 25 10:37:54 2001
* Modified:     Wed Jul  9 00:06:42 2003 (Eric Anderson) anderse@hpl.hp.com
* Language:     C++
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 2001, Hewlett-Packard Laboratories, all rights reserved.
*
*******************************************************************************
*/

#ifndef __MATRIX_H
#define __MATRIX_H

#include <LintelAssert.H>
#include <Double.H>

// gcc doesn't seem to be inlining the element stuff, and as a result, the
// performance is noticably slower for the table-based stuff.

#define BELIEVE_INLINE 0
#define MATRIX_CHECK_BOUNDS (COMPILE_DEBUG | DEBUG)
class Matrix {
public:
    Matrix(unsigned int rows, unsigned int cols);
    ~Matrix();
    void resize(unsigned int rows, unsigned int cols);

    inline void set(unsigned int row, unsigned int col, double value) {
#if BELIEVE_INLINE
	*elem(row,col) = value;
#else
#if MATRIX_CHECK_BOUNDS
	Assert(1,row < nrows && col < ncols);
#endif
	*(data + (row * ncols + col)) = value;
#endif	
    }
    // End all rows with Double::NaN; very important that things are clearly
    // doubles, e.g. setRow(5, 1.0,3,Double::NaN) won't work.
    void setRow(unsigned int row, ...);
    inline double get(unsigned int row, unsigned int col) {
#if BELIEVE_INLINE
	return *elem(row,col);
#else
#if MATRIX_CHECK_BOUNDS
	Assert(1,row < nrows && col < ncols);
#endif
	return *(data + (row * ncols + col));
#endif	
    }
    void setConstant(double value);
    // Returns number of columns that could be eliminated.
    // back substitution only occurs if we could eliminate all
    // of the columns up to maxcolumn, otherwise the matrix is left
    // as lower triangular.
    // maxcolumn == -1 => eliminate up to min(nrows,ncols)
    int gaussEliminate(int maxcolumn = -1);
    void swapRows(unsigned int row1, unsigned int row2);
    void divRow(unsigned int row, double val);
    void accumMultiply(unsigned int into_row, unsigned int from_row, 
		       double value);
    void print();
    static void selfTest();
private:
#if BELIEVE_INLINE
    inline double *elem(unsigned int row, unsigned int col) {
#if MATRIX_CHECK_BOUNDS
	Assert(1,row < nrows && col < ncols);
#endif
	return data + (row * ncols + col);
    }
#endif
    double *data;
    unsigned int nrows, ncols;
};

#endif
