#include <iostream>
#include <vector>
#include <string>

#include <Lintel/AssertBoost.hpp>
#include <Lintel/STLUtility.hpp>

using namespace std;
using boost::format;
using lintel::iteratorRangeEqual;

vector<string> first_range;
vector<string> second_range;

enum rangeComparator {
    InvalidEnumConstant = 0,
    EqualRangeComparator,
    UnEqualRangeComparator,
    FirstRangeLargerComparator,
    SecondRangeLargerComparator,
    FirstRangeNull
};

void prepareVectors(rangeComparator comparison_type, bool &expected_result, string &str_output) {
    if (comparison_type == EqualRangeComparator) {
        first_range.push_back("10");   
        first_range.push_back("20");   
        first_range.push_back("30"); 

        second_range.push_back("10");   
        second_range.push_back("20");   
        second_range.push_back("30");	

        expected_result = true;
        str_output = "Ranges with exact values and length";    
    }

    if (comparison_type == UnEqualRangeComparator) {
        first_range.push_back("10");   
        first_range.push_back("20");   
        first_range.push_back("30"); 
	
        second_range.push_back("10");   
        second_range.push_back("20");   
        second_range.push_back("20");
	
        expected_result = false;
        str_output = "Ranges with different values but same length";	    
    }

    if (comparison_type == FirstRangeLargerComparator) {
        first_range.push_back("10");   
        first_range.push_back("20");   
        first_range.push_back("30"); 
        first_range.push_back("40"); 
        first_range.push_back("50"); 
	
        second_range.push_back("20");   
        second_range.push_back("30");   
        second_range.push_back("40");
        second_range.push_back("50");
	
        expected_result = false;
        str_output = "Ranges with different values and lengths (second range subset of first range)";
    }
    
    if (comparison_type == SecondRangeLargerComparator) {	
        first_range.push_back("10");   
        first_range.push_back("20"); 
        first_range.push_back("30"); 
        first_range.push_back("40"); 
	
        second_range.push_back("10");   
        second_range.push_back("20");   
        second_range.push_back("30");   
        second_range.push_back("40");
        second_range.push_back("50");
	
        expected_result = false;	
        str_output = "Ranges with different values and lengths (first range subset of second range)";
    } 

    if (comparison_type == FirstRangeNull) {	
        second_range.push_back("10");   
        second_range.push_back("20");   
        second_range.push_back("30");   
        second_range.push_back("40");
        second_range.push_back("50");
	
        expected_result = false;	
        str_output = "First range empty and second range populated";
    } 
   
}

void clearRanges() {
    first_range.clear();
    second_range.clear();
}

int main() {    	
    bool retValue;
    string comparisonType;

    cout << "Range comparison tests begin \n";

    for (uint32_t i = EqualRangeComparator; i <= FirstRangeNull; i++) {
        prepareVectors((rangeComparator)i, retValue, comparisonType);
        INVARIANT(retValue == iteratorRangeEqual(first_range.begin(), first_range.end(), second_range.begin(),
                second_range.end()), format("Comparison type = %s failed.") % comparisonType);		
        clearRanges();
    }    
    cout << "Range comparison tests success!";
	
}
