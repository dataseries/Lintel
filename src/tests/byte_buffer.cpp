/* -*-C++-*- */
/*
   (c) Copyright 2009, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file

    Regression test.
*/

// TODO-eric: lots of tests.

#include <iostream>

#include <Lintel/ByteBuffer.hpp>
#include <Lintel/MersenneTwisterRandom.hpp>

using namespace std;
using boost::format;
using lintel::ByteBuffer;

void basicChecks(ByteBuffer &buf) {
    SINVARIANT(buf.empty());
    SINVARIANT(buf.bufferSize() == 0);
    SINVARIANT(buf.readAvailable() == 0);
    SINVARIANT(buf.writeAvailable() == 0);

    buf.resizeBuffer(8192);
    SINVARIANT(buf.readAvailable() == 0);
    SINVARIANT(buf.writeAvailable() == 8192);
    uint32_t *ibuf = buf.writeStartAs<uint32_t>(3072/4);
    for(uint32_t i = 0; i < 3072/4; ++i) {
	ibuf[i] = i;
    }
    SINVARIANT(buf.readAvailable() == 3072);
    SINVARIANT(buf.writeAvailable() == 8192 - 3072);
    ibuf = reinterpret_cast<uint32_t *>(buf.writeStart(8192-3072));
    for(uint32_t i = 0; i < (8192-3072)/4; ++i) {
	ibuf[i] = i + 3072/4;
    }
    SINVARIANT(buf.readAvailable() == 8192);
    SINVARIANT(buf.writeAvailable() == 0);
    for(uint32_t i = 0; i < 8192/4; ++i) {
	SINVARIANT(*buf.readStartAs<uint32_t>() == i);
	SINVARIANT(buf.writeAvailable() == 0);
	SINVARIANT(buf.readAvailable() == 8192 - i * 4);
	buf.consume(4);
    }
    SINVARIANT(buf.readAvailable() == 0 && buf.writeAvailable() == 0);
    buf.reset();
    SINVARIANT(buf.readAvailable() == 0 && buf.writeAvailable() == 8192);
    SINVARIANT(buf.bufferSize() == 8192);
    buf.purge();
    SINVARIANT(buf.readAvailable() == 0 && buf.writeAvailable() == 0 
	       && buf.bufferSize() == 0);
    buf.resizeBuffer(8);
    buf.writeStartAs<int64_t>(1)[0] = 0x123456789ABCDEFLL;
    buf.resizeBuffer(24);
    SINVARIANT(buf.writeAvailable() == 16);
    buf.writeStartAs<int64_t>(1)[0] = 0x234567891ABCDEFLL;
    SINVARIANT(*buf.readStartAs<int64_t>() == 0x123456789ABCDEFLL);
    buf.consume(8);
    SINVARIANT(buf.writeAvailable() == 8);
    SINVARIANT(*buf.readStartAs<int64_t>() == 0x234567891ABCDEFLL);
    buf.writeStartAs<int64_t>(1)[0] = 0x345678912ABCDEFLL;
    buf.shift();
    SINVARIANT(*buf.readStartAs<int64_t>() == 0x234567891ABCDEFLL);
    buf.consume(8);
    SINVARIANT(buf.readAvailable() == 8);
    buf.resizeBuffer(8);
    SINVARIANT(*buf.readStartAs<int64_t>() == 0x345678912ABCDEFLL);
    buf.consume(8);
    SINVARIANT(buf.empty());
    cout << "passed basic checks\n";
}

void randomChecks(ByteBuffer &buf) {
    vector<uint8_t> expected;
    uint32_t expected_at = 0;

    MersenneTwisterRandom mt;
    cout << format("seeded with %d\n") % mt.seed_used;
    SINVARIANT(buf.empty());
    uint32_t read_count = 0, write_count = 0, resize_count = 0, shift_count = 0;
    for(uint32_t i = 0; i < 10000; ++i) {
	uint32_t nbytes = mt.randInt() & 0xFF;
	if ((mt.randInt() & 0x1) == 0) { // write 
	    ++write_count;
	    if (buf.writeAvailable() < nbytes) {
		++shift_count;
		buf.shift();
		if (buf.writeAvailable() < nbytes) {
		    ++resize_count;
		    buf.resizeBuffer(buf.bufferSize() + nbytes);
		}
		SINVARIANT(buf.writeAvailable() >= nbytes);
	    }
	    uint8_t *write_at = buf.writeStart(nbytes);
	    for(uint32_t j = 0; j < nbytes; ++j) {
		uint8_t v = mt.randInt() & 0xFF;
		expected.push_back(v);
		write_at[j] = v;
	    }
	} else { // read
	    ++read_count;
	    if (buf.readAvailable() < nbytes) {
		nbytes = buf.readAvailable();
	    }
	    for(uint32_t j = 0; j < nbytes; ++j) {
		SINVARIANT(*buf.readStart() == expected[expected_at]);
		++expected_at;
		buf.consume(1);
	    }
	    if (buf.bufferSize() > 2 * buf.readAvailable()) {
		++resize_count;
		buf.resizeBuffer(buf.readAvailable());
	    }

	    if ((mt.randInt() & 0xF) < 2) { 
		++shift_count;
		buf.shift();
	    }
	}
	SINVARIANT(buf.readAvailable() == expected.size() - expected_at);
    }
    while(expected_at < expected.size()) {
	SINVARIANT(*buf.readStart() == expected[expected_at]);
	++expected_at;
	buf.consume(1);
    }
    cout << format("passed random checks over %d bytes:\n"
		   "  %d reads, %d writes, %d resizes, %d shifts\n") 
	% expected.size() % read_count % write_count % resize_count % shift_count;
}    

void copyChecks(ByteBuffer &buf) {
    SINVARIANT(buf.empty());

    string data("Hello, World.");
    buf.resizeBuffer(data.size());
    memcpy(buf.writeStart(data.size()), data.data(), data.size());
    SINVARIANT(buf.readAvailable() == data.size());
    
    ByteBuffer buf2 = buf;
    
    buf2.consume(5);
    SINVARIANT(buf.readAvailable() == data.size());
    SINVARIANT(buf2.readAvailable() == data.size() - 5);
    string data2(", Xorld.");
    buf2.writeableReadStart()[2] = 'X';
    SINVARIANT(buf.asString() == data);
    SINVARIANT(buf2.asString() == data2);
    cout << "passed copy checks\n";
}

int main() {
    ByteBuffer buf(true);

    basicChecks(buf);
    randomChecks(buf);
    copyChecks(buf);
    return 0;
}
