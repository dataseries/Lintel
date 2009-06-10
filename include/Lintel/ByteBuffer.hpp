/* -*-C++-*- */
#ifndef LINTEL_BYTEBUFFER_HPP
#define LINTEL_BYTEBUFFER_HPP
/*
   (c) Copyright 2009, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file 

    A class similar to a string, but able to be quickly initialized.
    The main problem with strings is that there is no efficient way to
    read from a file into a string, the string has to be constructed
    by calling assign or append from some other buffer resulting in
    multiple copies.
*/

#include <sys/types.h>

#include <string>
#include <iostream>

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include <Lintel/AssertBoost.hpp>

// WARNING: This class is used by thrift.  When modifying the class
// ensure that no changes are made to the structural layout of the
// classes (no virtual functions, no additional member values), or 
// else you will have to rebuild/package thrift. Thrift is not 
// rebuilt by deptool.

// TODO: consider using intrusive pointer to reduce the memory
// overhead of the ByteBuffer and number of allocations necessary.

// TODO: consider making the ability to perform the copy on write in a
// ByteBuffer part of the type.  Then there's no ambiguity in whether
// we can copy a ByteBuffer.  This will reduce the memory usage of a
// ByteBuffer.
namespace lintel {
    /// Basic buffer class.  Copies are disabled because they would be
    /// painfully inefficient.  If you want a copyable buffer, then
    /// use the standard ByteBuffer that does copy on write similar to
    /// how C++ strings are implemented.  See ByteBuffer for method
    /// documentation.
    class NoCopyByteBuffer : boost::noncopyable {
    public:
	NoCopyByteBuffer() : byte_data(NULL), data_size(0), front(0), back(0) { }
	~NoCopyByteBuffer() { delete [] byte_data; }

	bool empty() const {
	    return front == back;
	}

	size_t bufferSize() const {
	    return data_size;
	}

	size_t readAvailable() const {
	    return back - front;
	}

	const uint8_t *readStart() const {
	    return byte_data + front;
	}
	    
	template<typename T> const T *readStartAs() const {
	    return reinterpret_cast<const T *>(readStart());
	}
	void consume(size_t amt) {
	    DEBUG_SINVARIANT(amt <= readAvailable());
	    front += amt;
	}

	size_t writeAvailable() const {
	    return data_size - back;
	}

	uint8_t *writeStart() {
	    return byte_data + back;
	}

	template<typename T> T *writeStartAs() {
	    return reinterpret_cast<T *>(writeStart());
	}	    
	    
	void extend(size_t amt) {
	    DEBUG_SINVARIANT(amt <= writeAvailable());
	    back += amt;
	}

        void truncate(size_t amt = 0) {
	    DEBUG_SINVARIANT(amt >= 0 && amt <= readAvailable());
	    back = front + amt;
	}

	void resizeBuffer(uint32_t new_size) {
	    uint8_t *new_data = new uint8_t[new_size];
		
	    if (!empty()) {
		uint32_t old_available = readAvailable();
		SINVARIANT(old_available <= new_size);
		memcpy(new_data, readStart(), old_available);
		back = old_available;
	    } else {
		back = 0;
	    }
	    front = 0;
	    data_size = new_size;
	    delete [] byte_data;
	    byte_data = new_data;
	}

	void shift() {
	    if (empty()) {
		front = back = 0;
	    } else if (front > 0) {
		size_t old_size = readAvailable();
		memmove(byte_data, readStart(), old_size);
		front = 0;
		back = old_size;
	    }
	}

	void reset() {
	    front = back = 0;
	}

	void purge() {
	    delete [] byte_data;
	    byte_data = NULL;
	    data_size = front = back = 0;
	}
	
	std::string asString() const {
	    return std::string(readStartAs<char>(), readAvailable());
	}
    private:
	uint8_t *byte_data;
	// Data is valid for [data + front, data + back - 1], and has
	// size data_size.
	size_t data_size, front, back;
    };

    /// Copy on write buffer of bytes.  This is similar to a C++
    /// string, but data can be efficiently read into the buffer.
    /// With a string, the data first has to be read into a separate
    /// buffer and then copied into the string.
    class ByteBuffer {
    public:
	/// Construct a ByteBuffer.  If we allow copy on write, then
	/// when we perform mutating operations, if this is not a
	/// unique copy, then we copy the underlying buffer.  If it is
	/// not, then if there copies, mutations are not allowed
	/// (until all but one copy is destroyed). This is off by
	/// default on the assumption that it could be slow and is
	/// unlikely to be the desired behavior.  In no case does a
	/// copy produce a bit-for-bit copy of the buffer
	/// automatically, only a write after a copy does.  In no case
	/// will a write to one ByteBuffer cause a change in another.
	///
	/// This argument is poorly named, but nobody can think of a
	/// truly better name.  allow_mutable_copies was one
	/// suggestion, but doesn't capture that even when false, you
	/// can make copies, and then once all but one have been
	/// destroyed, you can continue making mutations.
	explicit ByteBuffer(bool allow_copy_on_write = false) 
	    : rep(new NoCopyByteBuffer()), allow_copy_on_write(allow_copy_on_write) { 
	}

	explicit ByteBuffer(const std::string& init) 
	    : rep(new NoCopyByteBuffer()), allow_copy_on_write(false) {
	    write(init);
	}

	explicit ByteBuffer(const char *str, int32_t len=-1) 
	    : rep(new NoCopyByteBuffer()), allow_copy_on_write(false) {
	    if (len < 0) {
		len = strlen(str);
	    }
	    write(str, len);
	}
#if 0
	ByteBuffer(const lintel::ByteBuffer &buf) 
	    : rep(buf.rep), allow_copy_on_write(buf.allow_copy_on_write) {
	    if (!allow_copy_on_write) {
		const_cast<ByteBuffer&>(buf).disconnect();
	    }
	}
#endif

	/// true if there are no bytes in the buffer
	bool empty() const {
	    return rep->empty();
	}

	/// how large is the buffer overall, e.g what is the current
	/// maximum value for readAvailable() and writeAvailable().  
	/// Note that readAvailable() + writeAvailable() <= bufferSize()
	size_t bufferSize() {
	    return rep->bufferSize();
	}

	/// number of bytes available for reading in the buffer
	size_t readAvailable() const {
	    return rep->readAvailable();
	}
	
	/// pointer to the start of the read data, valid for
	/// readAvailable() bytes.
	const uint8_t *readStart() const {
	    return rep->readStart();
	}

	/// pointer to the start of the read data, interpreted as a
	/// particular type.  Note that there is no guarantee that the
	/// read buffer is properly aligned with the type unless you
	/// have previously called shift().
	template<typename T> const T *readStartAs() const {
	    return rep->readStartAs<T>();
	}

	/// remove bytes from being readable, readAvailable() is
	/// reduced by amt.  Invalid to call with amt >
	/// readAvailable().
	void consume(size_t amt) {
	    uniqueify();
	    rep->consume(amt);
	}

	/// number of bytes available to be written without having to
	/// resize the buffer.
	size_t writeAvailable() const {
	    return rep->writeAvailable();
	}

	/// pointer to the start of where data could be written.
	/// Valid to be written for up to writeAvailable() bytes.
	uint8_t *writeStart() {
	    uniqueify();
	    return rep->writeStart();
	}

	/// writes buf contents into this buffer.
	void write(const ByteBuffer &buf) {
	    write( buf.readStart(), buf.readAvailable());
	}

	/// writes string contents to the bytebuffer
	void write(const std::string& str) {
	    write( str.data(), str.size());
	}

	/// writes character data to the bytebuffer, resizing if needed
	void write(const void *buf, int32_t size) {
	    uniqueify();
	    SINVARIANT(size >= 0);
	    if (writeAvailable() < (size_t)size) {
	        int needed = size - writeAvailable();
		resizeBuffer( bufferSize() + needed);
	    }
	    memcpy(writeStart(), buf, size);
	    extend(size);
	}

	/// writes bytes to the bytebuffer, resizing if needed
	void pad(char b, int32_t size) {
	    uniqueify();
	    SINVARIANT(size >= 0);
	    if (writeAvailable() < (size_t)size) {
	        int needed = size - writeAvailable();
		resizeBuffer( bufferSize() + needed);
	    }
	    memset(writeStart(), b, size);
	    extend(size);
	}

	/// 
	void trunc_or_pad(int32_t size, char b = 0) {
	    SINVARIANT(size >= 0);
	    if (readAvailable() >= (unsigned)size) {
		uniqueify(size == 0);
	        rep->truncate(size);
	    } else {
		pad( b, size);
	    }
	}

	/// replace buffer contents from content of a byte array
        void replace(int32_t offset, const void *src, int32_t length) {
	    uniqueify();
	    SINVARIANT(length >= 0);
	    SINVARIANT(static_cast<uint32_t>(offset + length) < readAvailable());
	    memcpy(const_cast<uint8_t *>(readStart())+offset, src, length);
	}

	template<typename T> T *writeStartAs() {
	    uniqueify();
	    return rep->writeStartAs<T>();
	}

	/// after bytes are written into the array, you can extend the
	/// array to cover the bytes that have been added into the
	/// array.  Invalid to call with amt > writeAvailable()
	void extend(size_t amt) {
	    uniqueify();
	    rep->extend(amt);
	}

	/// Resize the bufer to be new_size bytes in length.  It is an
	/// error to call this with readAvailable() > new_size.
	/// Currently readable data will be moved to the beginning of
	/// the buffer.
	void resizeBuffer(uint32_t new_size) {
	    uniqueify();
	    rep->resizeBuffer(new_size);
	}

	/// Shift the data in the buffer to the begining of the buffer.
	void shift() {
	    uniqueify();
	    rep->shift();
	}

	/// Reset the buffer, empties it out.
	void reset() {
	    uniqueify();
	    rep->reset();
	}

	/// Purge the buffer, this sets the buffer size to 0 and frees
	/// the associated data.
	void purge() {
	    uniqueify(true);
	    rep->purge();
	}

	/// Place a new value into the ByteBuffer throwing away any
	/// previous contents.
	void assign( const ByteBuffer& buf) {
	    purge();
	    write(buf);
	}
	
	/// Place a new string into the ByteBuffer throwing away any
	/// previous content.
	void assign( const std::string& src) {
	    purge();
	    write(src);
	}

	/// Place the specified bytes into the ByteBuffer, throwing away
	/// any previous contents.
	void assign( const void * src, int32_t len) {
	    purge();
	    write(src, len);
	}

	/// Disconnect from the buffer.  If there are no other references
	/// the buffer will be deleted.  The buffer is no longer usable
	/// after a call to disconnect()
	void disconnect() {
	    rep.reset();
	}

	/// Force the underlying buffer to be released and reallocated.  
	/// If 'purge' is true the new buffer will be empty, if false 
	/// its contents will be copied from the old buffer before the
	/// old buffer is released.
	void reinit( bool purge=true) {
	    NoCopyByteBuffer *new_buf = new NoCopyByteBuffer();
	    if (!purge) {
		new_buf->resizeBuffer(rep->readAvailable());
		memcpy(new_buf->writeStart(), rep->readStart(), rep->readAvailable());
		new_buf->extend(rep->readAvailable());
	    }
	    rep.reset(new_buf); // release the old buffer, attach the new
	}

	/// Return the available part of the read buffer as a string
	std::string asString() const {
	    return rep->asString();
	}

	bool operator==(const ByteBuffer& rhs) const {
	    if (readAvailable() != rhs.readAvailable()) return false;
	    if (memcmp(readStart(), rhs.readStart(), readAvailable()) != 0) {
		return false;
	    }
	    return true;
	}

	bool operator!=(const ByteBuffer& rhs) const {
	    return !(*this == rhs);
	}

    private:
	void uniqueify( bool purge=false) {
	    if (!rep.unique()) {
		SINVARIANT(allow_copy_on_write);
		reinit( purge);
	    }
	}

	boost::shared_ptr<NoCopyByteBuffer> rep;
	bool allow_copy_on_write;
    };
}
#endif
