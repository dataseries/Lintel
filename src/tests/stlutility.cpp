#include <iostream>
#include <vector>
#include <string>

#include <Lintel/AssertBoost.hpp>
#include <Lintel/STLUtility.hpp>

using namespace std;
using boost::format;
using lintel::iteratorRangeEqual;

vector<string> &operator<<(vector<string> &lhs, const string &rhs) {
    lhs.push_back(rhs);
    return lhs;
}

void check(const vector<string> &first, const vector<string> &second,
	   const std::string &test_name, bool expected) {
    INVARIANT(expected == 
	      iteratorRangeEqual(first.begin(), first.end(), second.begin(), second.end()),
	      format("%s test failed") % test_name);
    cout << format("%s test passed\n") % test_name;
}
	      
void testIdentical() {
    vector<string> first, second;

    first  << "10" << "20" << "30";
    second << "10" << "20" << "30";

    check(first, second, "Identical ranges", true);
}

void testUnequal() {
    vector<string> first, second;
    
    first  << "10" << "20" << "30";
    second << "10" << "20" << "20";

    check(first, second, "Unequal values", false);
}

void testFirstLarger() {
    vector<string> first, second;
    
    first  << "10" << "20" << "30" << "40" << "50";
    second << "10" << "20" << "30" << "40";

    check(first, second, "First range larger", false);
}

void testSecondLarger() {
    vector<string> first, second;
    
    first  << "10" << "20" << "30" << "40";
    second << "10" << "20" << "30" << "40" << "50";

    check(first, second, "Second range larger", false);
}
    
void testFirstNull() {
    vector<string> first, second;
    
    second << "10";

    check(first, second, "First range null", false);
}

void testSecondNull() {
    vector<string> first, second;
    
    first << "10";

    check(first, second, "Second range null", false);
}

void testBothNull() {
    vector<string> first, second;
    
    check(first, second, "Both range null", true);
}

int main() {    	
    cout << "Range comparison tests begin \n";

    testIdentical();
    testUnequal();
    testFirstLarger();
    testSecondLarger();
    testFirstNull();
    testSecondNull();
    testBothNull();

    cout << "Range comparison tests success!";
	
}
