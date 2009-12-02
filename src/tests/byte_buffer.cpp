/* -*-C++-*- */
/*
   (c) Copyright 2009, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file

    Regression test.
*/

#include <inttypes.h>
#include <iostream>
#include <fstream>
#include <sstream>
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
    uint32_t *ibuf = buf.writeStartAs<uint32_t>(1024/4);
    for(uint32_t i = 0; i < 3072/4; ++i) {
	ibuf[i] = i;
    }
    buf.extend(3072-1024);
    SINVARIANT(buf.readAvailable() == 3072);
    SINVARIANT(buf.writeAvailable() == 8192 - 3072);
    for(uint32_t i = 0; i < (8192-3072)/4; ++i) {
	reinterpret_cast<uint32_t *>(buf.writeStart(0))[0] = i + 3072/4;
	buf.extendAs<uint32_t>(1);
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

void constructorTests() {
    ByteBuffer buf1(string("abcdef"));

    SINVARIANT(buf1.asString() == "abcdef");
    
    ByteBuffer buf2("xyzzy\0abc", 9);
    
    SINVARIANT(buf2.asString() == string("xyzzy\0abc", 9));

    ByteBuffer buf3("foo");
    SINVARIANT(buf3.asString() == "foo");

    ByteBuffer buf4(buf3);
    SINVARIANT(buf4.asString() == "foo");

    ByteBuffer buf5 = buf4;
    SINVARIANT(buf5.asString() == "foo");
    
    cout << "passed constructor checks\n";
}

void appendReplaceAssignTests() {
    ByteBuffer buf1;

    buf1.append("abc");
    buf1.append("def");

    SINVARIANT(buf1.asString() == "abcdef");

    ByteBuffer buf2("123");
    buf2.append(buf1);

    SINVARIANT(buf2.asString() == "123abcdef");

    buf2.append("456789", 2);
    SINVARIANT(buf2.asString() == "123abcdef45");

    buf2.replace(1, "AZ", 2);
    SINVARIANT(buf2.asString() == "1AZabcdef45");

    ByteBuffer buf3("zzz");
    SINVARIANT(buf3.asString() == "zzz");
    buf3.assign(buf1);
    SINVARIANT(buf3.asString() == "abcdef");

    buf1.assign("bbb");
    SINVARIANT(buf1.asString() == "bbb");

    buf2.assign("12345", 3);
    SINVARIANT(buf2.asString() == "123");

    SINVARIANT(buf2 != buf1);

    buf2.assign(buf1);
    SINVARIANT(buf1 == buf2);

    ByteBuffer buf4(buf2);
    SINVARIANT(buf4 == buf2 && buf4 == buf1);

    buf4.forceUnique();
    buf4.append("aa");
    SINVARIANT(buf4 != buf2);
    SINVARIANT(buf4.asString() == "bbbaa");

    cout << "passed append/replace/assign checks\n";
}

void appendStreamTests(int argc, char *argv[]) {

    SINVARIANT(argc == 2);
    char *filename = argv[1];

    ByteBuffer buf;

    int64_t bread;
    
    ifstream sin(filename, ios_base::in);
    SINVARIANT(!sin.fail());
    bread = buf.appendEntireStream(sin);
    SINVARIANT(sin.eof());
    INVARIANT(bread == 108, format("Read on %d bytes, expected 108") % bread);

    // Read in the file a couple of times, make sure it appends.
    SINVARIANT(buf.bufferSize() == 1024);
    INVARIANT(buf.asString() ==
	      "This is a test of the Lintel ByteBuffer append stream methods.\n"
	      "Its as complete as its going to get for now.\n",
	      buf.asString());

    sin.clear();
    sin.seekg(0, ios::beg);
    SINVARIANT(!(sin.eof() || sin.fail() || sin.bad()));
    bread = buf.appendEntireStream(sin);
    SINVARIANT(sin.eof());
    INVARIANT(bread == 108, format("Read on %d bytes, expected 108") % bread);
    
    INVARIANT(buf.asString() ==
	      "This is a test of the Lintel ByteBuffer append stream methods.\n"
	      "Its as complete as its going to get for now.\n"
	      "This is a test of the Lintel ByteBuffer append stream methods.\n"
	       "Its as complete as its going to get for now.\n",
	       buf.asString());

    // Read in more than the buffer size, make sure it automatically
    // resizes. The buffer ends up being 1024 bytes, after the first read. We
    // have read the file in twice. Reading it in 12 more times will force the
    // buffer to resize again up to 2048.
    for (int i = 0; i < 12; i++) {
	sin.clear();
	sin.seekg(0, ios::beg);
	SINVARIANT(!(sin.eof() || sin.fail() || sin.bad()));
	buf.appendEntireStream(sin);
	SINVARIANT(sin.eof());
    }
    SINVARIANT(buf.bufferSize() == 2048);
    
    // file = 108 bytes. We've appended a total of 14 times: 14*108=1512.
    INVARIANT(buf.readAvailable() == 1512,
	      format("Read available %d, expected 1512") % buf.readAvailable());

    // Exercise the max_size argument, first with something in between doubling
    // and more than what's read ...
    
    buf.reset();
    buf.resizeBuffer(0);
    
    for (int i = 0; i < 14; i++) {
	sin.clear();
	sin.seekg(0, ios::beg);
	SINVARIANT(!(sin.eof() || sin.fail() || sin.bad()));
	buf.appendEntireStream(sin, 1600);
	SINVARIANT(sin.eof());
    }

    SINVARIANT(buf.bufferSize() == 1600);
    INVARIANT(buf.readAvailable() == 1512,
	      format("Read available %d, expected 1512") % buf.readAvailable());

    buf.reset();
    buf.resizeBuffer(0);

    // then with something between doubling the size and less than what's read.
    for (int i = 0; i < 14; i++) {
	sin.clear();
	sin.seekg(0, ios::beg);
	SINVARIANT(!(sin.eof() || sin.fail() || sin.bad()));
	buf.appendEntireStream(sin, 1500);
    }
    
    SINVARIANT(!sin.eof());
    SINVARIANT(buf.bufferSize() == 1500);
    INVARIANT(buf.readAvailable() == 1500,
	      format("Read available %d, expected 1512") % buf.readAvailable());
    
    buf.reset();
    buf.resizeBuffer(0);

    // Read in partial/fixed amounts and check edge conditions near EOF.
    sin.seekg(0, ios::beg);
    sin.clear();
    SINVARIANT(!(sin.eof() || sin.fail() || sin.bad()));
    buf.appendFromStream(sin, 8);
    SINVARIANT(!sin.eof());
    SINVARIANT(buf.asString() == "This is ");
    buf.resizeBuffer(108);
    buf.appendFromStream(sin, 99);
    SINVARIANT(!sin.eof());
    buf.appendFromStream(sin, 1);
    SINVARIANT(!sin.eof());
    buf.appendFromStream(sin, 1);
    SINVARIANT(sin.eof());
    SINVARIANT(buf.bufferSize() == 109);
    INVARIANT(buf.asString() ==
	      "This is a test of the Lintel ByteBuffer append stream methods.\n"
	      "Its as complete as its going to get for now.\n",
	      buf.asString());
    sin.close();
}

void overloadOutOperatorTests() {
    string test_str("Hello, World.");
    const ByteBuffer buff(test_str);
    ostringstream osstream;
    osstream << buff;
    SINVARIANT(osstream.str() == test_str);
}

void unextendTest() {
    ByteBuffer buff("0123456789");
    SINVARIANT(buff.readAvailable() == 10);
    SINVARIANT(buff.writeAvailable() == 0);
    buff.unextend(5);
    SINVARIANT(buff.readAvailable() == 5);
    SINVARIANT(buff.writeAvailable() == 5);
    SINVARIANT(buff.asString() == "01234");
}

int main(int argc, char *argv[]) {
    ByteBuffer buf(true);

    basicChecks(buf);
    randomChecks(buf);
    copyChecks(buf);
    constructorTests();
    appendReplaceAssignTests();
    appendStreamTests(argc, argv);
    overloadOutOperatorTests();
    unextendTest();
    
    return 0;
}
