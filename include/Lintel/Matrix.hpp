/* -*-C++-*- */
/*
   (c) Copyright 2001-2005, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    \brief header file for Matrix class
*/

#ifndef LINTEL_MATRIX_HPP
#define LINTEL_MATRIX_HPP

#include <Lintel/Double.hpp>
#include <Lintel/AssertBoost.hpp>

// gcc doesn't seem to be inlining the element stuff, and as a result, the
// performance is noticably slower for the table-based stuff.

#define BELIEVE_INLINE 0
/// \brief Various useful matrix manipulation functions
class Matrix {
public:
    Matrix(unsigned int rows, unsigned int cols);
    ~Matrix();
    void resize(unsigned int rows, unsigned int cols);

    inline void set(unsigned int row, unsigned int col, double value) {
#if BELIEVE_INLINE
	*elem(row,col) = value;
#else
	DEBUG_SINVARIANT(row < nrows && col < ncols);
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
	DEBUG_SINVARIANT(row < nrows && col < ncols);
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
	DEBUG_SINVARIANT(row < nrows && col < ncols);
	return data + (row * ncols + col);
    }
#endif
    double *data;
    unsigned int nrows, ncols;
};

#endif
