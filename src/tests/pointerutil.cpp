/* -*-C++-*- */
/*
   (c) Copyright 2008, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file
    Tests for PointerUtil
*/

#include <iostream>

#include <boost/cast.hpp>
using namespace boost;

#include <Lintel/PointerUtil.hpp>

using namespace std;
using namespace Lintel;
using boost::shared_ptr;
using boost::format;

class base1 {
public:
    typedef shared_ptr<base1> Ptr;
    base1(int a) : val1(a) { }
    virtual ~base1() { }

    int val1;
};

class base2 {
public:
    typedef shared_ptr<base2> Ptr;
    base2(int a) : val2(a) { }
    virtual ~base2() { }

    int val2;
};

class derived1 : public base1, public base2  {
public:
    typedef shared_ptr<derived1> Ptr;

    derived1(int a, int b) : base1(a), base2(b) { }
    virtual ~derived1() { }
};

class derived2 : public base1, public base2  {
public:
    typedef shared_ptr<derived2> Ptr;

    derived2(int a, int b, int c) : base1(a), base2(b), val3(c) { }
    virtual ~derived2() { }

    int val3;
};

class unrelated {
public:
    typedef shared_ptr<unrelated> Ptr;

    unrelated(int a) : val3(a) { }
    virtual ~unrelated() { }

    int val3;
};

int main()
{
    derived1 *d1 = new derived1(3,4);

    base1 *d1_b1 = static_cast<base1 *>(d1);
    base2 *d1_b2 = d1;

    SINVARIANT(safe_downcast<derived1>(d1_b1) == d1);
    SINVARIANT(safe_downcast<derived1>(d1_b2) == d1);

    SINVARIANT(safe_downcast<base1>(d1) == d1_b1); // upcast
    SINVARIANT(safe_downcast<base2>(d1) == d1_b2); // upcast

    SINVARIANT(safe_crosscast<base2>(d1_b1) == d1_b2);
    SINVARIANT(safe_crosscast<base1>(d1_b2) == d1_b1);

    AssertBoostFnBefore(AssertBoostThrowExceptionFn);
    try {
	SINVARIANT(safe_downcast<derived2>(d1_b1) == 0);
	FATAL_ERROR("?");
    } catch (AssertBoostException &e) {
	SINVARIANT(e.msg == "dynamic downcast failed in Target* Lintel::safe_downcast(Source*) [with Target = derived2, Source = base1]");
    }

    try {
	SINVARIANT(safe_crosscast<unrelated>(d1_b1) == 0);
	FATAL_ERROR("?");
    } catch (AssertBoostException &e) {
	SINVARIANT(e.msg == "dynamic crosscast failed in Target* Lintel::safe_crosscast(Source*) [with Target = unrelated, Source = base1]");
    }

    // Once again with shared pointers ...

    derived1::Ptr p_d1(d1);
    
    base1::Ptr p_d1_b1 = static_pointer_cast<base1>(p_d1);
    base2::Ptr p_d1_b2 = p_d1;

    SINVARIANT(safe_downcast<derived1>(p_d1_b1) == p_d1);
    SINVARIANT(safe_downcast<derived1>(p_d1_b2) == p_d1);

    SINVARIANT(safe_downcast<base1>(p_d1) == p_d1_b1); // upcast
    SINVARIANT(safe_downcast<base2>(p_d1) == p_d1_b2); // upcast

    SINVARIANT(safe_crosscast<base2>(p_d1_b1) == p_d1_b2);
    SINVARIANT(safe_crosscast<base1>(p_d1_b2) == p_d1_b1);

    try {
	SINVARIANT(safe_downcast<derived2>(p_d1_b1) == 0);
	FATAL_ERROR("?");
    } catch (AssertBoostException &e) {
	SINVARIANT(e.msg == "dynamic downcast failed in boost::shared_ptr<T> Lintel::safe_downcast(boost::shared_ptr<U>) [with Target = derived2, Source = base1]");
    }

    try {
	SINVARIANT(safe_crosscast<unrelated>(p_d1_b1) == 0);
	FATAL_ERROR("?");
    } catch (AssertBoostException &e) {
	SINVARIANT(e.msg == "dynamic crosscast failed in boost::shared_ptr<T> Lintel::safe_crosscast(boost::shared_ptr<U>) [with Target = unrelated, Source = base1]");
    }

    return 0;
}
