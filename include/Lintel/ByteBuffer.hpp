/* -*-C++-*- */
#ifndef LINTEL_BYTEBUFFER_HPP
#define LINTEL_BYTEBUFFER_HPP
/*
   (c) Copyright 2009, Hewlett-Packard Development Company, LP

   See the file named COPYING for license details
*/

/** @file 

    A class that represents an array of bytes that can be efficiently
    initialized.  The main problem with strings is that there is no efficient
    way to read from a file into a string, the string has to be constructed by
    calling assign or append from some other buffer resulting in multiple
    copies.
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
	    
	uint8_t *writeableReadStart() {
	    return byte_data + front;
	}

	void consume(size_t amt) {
	    DEBUG_SINVARIANT(amt <= readAvailable());
	    front += amt;
	}

	size_t writeAvailable() const {
	    return data_size - back;
	}

	uint8_t *writeStart(size_t extend_amt) {
	    uint8_t *ret = byte_data + back;
	    extend(extend_amt);
	    return ret;
	}

	void extend(size_t amt) {
	    SINVARIANT(amt <= writeAvailable());
	    back += amt;
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
	
    private:
	uint8_t *byte_data;
	// Data is valid for [data + front, data + back - 1], and has
	// size data_size.
	size_t data_size, front, back;
    };

    /// Copy on write buffer of bytes.  Unlike C++ strings, data can be
    /// efficiently read into the buffer.  With a string, the data first has to
    /// be read into a separate buffer and then copied into the string.
    class ByteBuffer {
    public:
	/// Construct a ByteBuffer.
	///
	/// A ByteBuffer is unique in two cases, (1) if it never has been copied
	/// through the assignment operator, or (2) if only one copy
	/// remains. Otherwise, there are multiple copies, so it is not unique.
	///
	/// allow_copy_on_write specifies what happens when the ByteBuffer is
	/// *not* unique. If true, like strings, a mutating operation will make
	/// a duplicate of the underlying buffer and mutate that. If false,
	/// unlike strings, then mutations are an error.
	///
	/// By default, we set this to false, on the assumption that it could be
	/// slow and is unlikely to be the desired behavior. In no case does a
	/// copy produce a bit-for-bit copy of the buffer automatically, only a
	/// write after a copy does.  In no case will a write to one ByteBuffer
	/// cause a change in another.
	///
	/// This flag could also be called enable_copy_when_not_unique.
	///
	explicit ByteBuffer(bool allow_copy_on_write = false) 
	    : rep(new NoCopyByteBuffer()), allow_copy_on_write(allow_copy_on_write) { 
	}

	/// Create a ByteBuffer and initialize it with the contents of @param init
	///
	/// @param init string to initialize with
	/// @param allow_copy_on_write should we allow this byte buffer to be copied if needed?
	explicit ByteBuffer(const std::string &init, bool allow_copy_on_write = false) 
	    : rep(new NoCopyByteBuffer()), allow_copy_on_write(allow_copy_on_write) {
	    append(init);
	}

	/// Create a ByteBuffer and initialize it with the contents of @param init
	/// If @param len < 0, automatically call strlen on init to get the length.
	///
	/// @param init character array to initialize with
	/// @param len length of the character array, or < 0 to use strlen to determine length
	/// @param allow_copy_on_write should we allow this byte buffer to be copied if needed?
	explicit ByteBuffer(const char *init, ssize_t len = -1, bool allow_copy_on_write = false) 
	    : rep(new NoCopyByteBuffer()), allow_copy_on_write(allow_copy_on_write) {
	    if (len < 0) {
		len = strlen(init);
	    }
	    append(init, len);
	}

	/// Copy constructor
	ByteBuffer(const ByteBuffer &from) 
	    : rep(from.rep), allow_copy_on_write(from.allow_copy_on_write)
	{ }

	/// Assignment operator
	ByteBuffer &operator =(const ByteBuffer &rhs) {
	    rep = rhs.rep;
	    allow_copy_on_write = rhs.allow_copy_on_write;
	    return *this;
	}

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
	    return reinterpret_cast<const T *>(readStart());
	}

	/// writeable pointer to the start of the read data, valid to be written
	/// for up to readAvailable() + writeAvailable() bytes. If you write
	/// after readAvailable(), then you need to extend().  Useful for
	/// constructing a buffer and then manipulating it, for example,
	/// updating a size near the front of the buffer.
	uint8_t *writeableReadStart() {
	    uniqueify();
	    return rep->writeableReadStart();
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

	/// pointer to the start of where @param extend_amt bytes of
	/// data can be written.  Valid to be written for up to
	/// writeAvailable() bytes.  Invalid to call with extend_amt >
	/// writeAvailable().  Readable region will be increased by
	/// extend_amt after this call.  Normally you would just use
	/// this method, but if you don't know exactly how many bytes
	/// will arrive, you can use writeStart(0) + extend, e.g. a
	/// call to read() on a network socket.
	///
	/// @param extend_amt how many bytes will be written. Hence
	/// readAvailable() will increase by extend_amt.
	uint8_t *writeStart(size_t extend_amt) {
	    uniqueify();
	    return rep->writeStart(extend_amt);
	}

	/// pointer to the start of where data could be written, as
	/// interpreted a a pointer to T.  Valid to be written for up
	/// to writeAvailable() / sizeof(T) elements.  Invalid to call
	/// with extend_elements * sizeof(T) > writeAvailable().
	/// Readable region will be increased by sizeof(T) *
	/// extend_elements.
	///
	/// @param extend_elements how many elements will be written.
	/// Hence readAvailable() will increase by sizeof(T) *
	/// extend_elements.
	template<typename T> T *writeStartAs(size_t extend_elements) {
	    return reinterpret_cast<T *>(writeStart(extend_elements * sizeof(T)));
	}

	/// after bytes are written into the array, you can extend the
	/// array to cover the bytes that have been added into the
	/// array.  Invalid to call with amt > writeAvailable().  
	void extend(size_t amt) {
	    uniqueify();
	    rep->extend(amt);
	}

	/// The same as extend except it counts in units of elements
	/// rather than bytes.
	template<typename T> void extendAs(size_t extend_elements) {
	    extend(sizeof(T) * extend_elements);
	}


	/// append contents of @param buf to this buffer, resizing if necessary.
	void append(const ByteBuffer &buf) {
	    append(buf.readStart(), buf.readAvailable());
	}

	/// append contents of @param str to this buffer, resizing if necessary.
	void append(const std::string &str) {
	    append(str.data(), str.size());
	}

	/// appends contents of @param buf with length @param size to this
	/// buffer, resizing if necessary.
	///
	/// @param buf buffer for appending to the byte buffer
	/// @param size length of buf
	void append(const void *buf, size_t size) {
	    uniqueify();
	    SINVARIANT(size >= 0);
	    if (writeAvailable() < size) {
	        int needed = size - writeAvailable();
		resizeBuffer( bufferSize() + needed);
	    }
	    memcpy(writeStart(size), buf, size);
	}

	/// appends data from @param sin until @param amount bytes read.
	///   If @param amount == -1, reads until eof.
	/// returns bytes read and -1 if error.
	///
	/// Increases buffer size to consume the @param amount bytes.
	///  If @param amount == -1 increases size exponentially to consume stream.
	/// @param max_size is the largest the buffer will get. Default @param
	/// max_size is 128MiB

	
	size_t appendFromStream(std::istream &sin, int32_t amount = -1,
				size_t max_size = 128*1024*1024) {

	    size_t bytes_read = 0;

	    SINVARIANT((amount >= -1) && (static_cast<size_t>(amount) < max_size));
	    
	    if ((amount != -1) && (writeAvailable() <
				   static_cast<size_t>(amount))) {
		resizeBuffer( bufferSize() + amount - writeAvailable() );
	    } else if (bufferSize() == 0) {
		resizeBuffer(1024);
	    }
	    

	    while ((!(sin.eof() || sin.fail() || sin.bad())) &&
		   ((amount == -1) || (bytes_read <
				       static_cast<size_t>(amount)))) {
		
		size_t to_read =
		    ((amount == -1) ||
		     (amount - bytes_read) > writeAvailable()) ?
		    writeAvailable() : (amount - bytes_read);

		if (to_read > 0) {
		    sin.read(writeStartAs<char>(0), to_read);
		    bytes_read += sin.gcount();
		    extend(sin.gcount());
		} else if ((bufferSize()* 2) < max_size) {
		    resizeBuffer(bufferSize()*2);
		}
	    }

	    if (sin.bad()) return (-1);

	    return bytes_read;
	}
	
	/// replace part of ByteBuffer contents starting at @param
	/// offset with @param src for @param length bytes.  
	/// Otherwise, offset + length must be < readAvailable().
	/// 
	/// @param offset start offset in the byte array for replacing
	/// @param src source data for replacing
	/// @param length length of data for replacing
        void replace(size_t offset, const void *src, size_t length) {
	    SINVARIANT(offset + length <= readAvailable());
	    memcpy(writeableReadStart() + offset, src, length);
	}


	/// Resize the bufer to be new_size bytes in length.  It is an
	/// error to call this with readAvailable() > new_size.
	/// Currently readable data will be moved to the beginning of
	/// the buffer.  Note this function is called resizeBuffer
	/// rather than just resize because it does *not* zero the
	/// buffer as is done in all of the STL resize functions.
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
	    uniqueify();
	    rep->purge();
	}

	/// Copy the value in @param src into this ByteBuffer,
	/// overwriting any prior contents.
	void assign(const ByteBuffer &src) {
	    reset();
	    append(src.readStart(), src.readAvailable());
	}
	
	/// Copy the value in @param src into this ByteBuffer,
	/// overwriting any prior contents.
	void assign(const std::string &src) {
	    reset();
	    append(src);
	}

	/// Copy the value in @param src for @param len bytes into
	/// this ByteBuffer, overwriting any prior contents.
	void assign(const void *src, size_t len) {
	    reset();
	    append(src, len);
	}

	/// Return the available part of the read buffer as a string
	std::string asString() const {
	    return std::string(readStartAs<char>(), readAvailable());
	}

	
	/// Compare two byte buffers for equality.
	bool operator==(const ByteBuffer &rhs) const {
	    if (readAvailable() != rhs.readAvailable()) {
		return false;
	    }
	    if (rep == rhs.rep) {
		return true;
	    }
	    return memcmp(readStart(), rhs.readStart(), readAvailable()) == 0;
	}

	/// Compare two byte buffers for inequality
	bool operator!=(const ByteBuffer& rhs) const {
	    return !(*this == rhs);
	}

	/// Force this ByteBuffer to be unique; note that this
	/// function can be slow because it will make a copy of the
	/// underlying buffer.
	void forceUnique() {
	    if (!rep.unique()) {
		NoCopyByteBuffer *new_buf = new NoCopyByteBuffer();
		new_buf->resizeBuffer(rep->readAvailable());
		memcpy(new_buf->writeStart(rep->readAvailable()), 
		       rep->readStart(), rep->readAvailable());
		rep.reset(new_buf);
	    }
	}	    

    private:
	void uniqueify() {
	    if (!rep.unique()) {
		SINVARIANT(allow_copy_on_write);
		
		forceUnique();
	    }
	}

	boost::shared_ptr<NoCopyByteBuffer> rep;
	bool allow_copy_on_write;
    };
}

#endif

