#include <iostream>
#include <Lintel/Base64.hpp>
#include <Lintel/LintelLog.hpp>
#include <Lintel/MersenneTwisterRandom.hpp>
#include <Lintel/TestUtil.hpp>

using namespace std;
using lintel::ASCIIbeticalB64;
using boost::format;

// Simple test added to verify Base64 basic encode, decode interface are working. There was a
// bug(typo) due to which this test is added.
int main(int argv, char *argc[]) {
    LintelLog::info("ASCIIbeticalB64 encoding decoding test starts");
    string data;
    if (argv == 1) {
        MersenneTwisterRandom rand;
        const static string hex_string = "0123456789abcde";
        uint32_t length = rand.randInt(64);
        for (uint32_t i = 0; i < length; ++i) { 
            char ch = hex_string[rand.randInt(16)];
            data.append(1, ch);
        }
    } else if (argv == 2) {
        data = argc[1];
    } else {
        FATAL_ERROR(format("This test accepts max. 1 argument but passed %d") % argv);
    }

    string encoded_data = ASCIIbeticalB64::encode(data.data(), data.size(), 0);
    string decoded_data = ASCIIbeticalB64::decode(encoded_data);

    LintelLog::info(format("Original data: \t%s") % data);
    LintelLog::info(format("Encoded data:  \t%s") % encoded_data);
    LintelLog::info(format("Decoded data:  \t%s") % decoded_data);

    SINVARIANT(data == decoded_data);
    LintelLog::info("Test passed");
}
