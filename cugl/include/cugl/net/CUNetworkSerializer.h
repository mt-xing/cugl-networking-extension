#ifndef CU_NETWORK_SERIALIZER_H
#define CU_NETWORK_SERIALIZER_H

#include <cstdint>
#include <string>
#include <vector>
#include <variant>
#include <cugl/assets/CUJsonValue.h>

namespace cugl {
	/**
	 * Helper class that serializes complex data into a byte array.
	 * 
	 * Intended for use with cugl::NetworkConnection.
	 * 
	 * Can serialize the following data types:
	 *  - Floats
	 *  - Doubles
	 *  - 32 Bit Signed + Unsigned Integers
	 *  - 64 Bit Signed + Unsigned Integers
	 *  - Strings (see note below)
	 *  - JsonValue (the cugl JSON class)
	 *  - Vectors of all above types
	 * 
	 * Note about Strings: if a char* is written, it will be deserialized as a std::string.
	 * The same applies to vectors of char*.
	 * 
	 * Deserialize these messages using NetworkDeserializer.
	 */
	class NetworkSerializer {
	public:

#define WRITE_METHODS(T, n) \
/** \
Write a single value. \
<p>\
Values will be deserialized on other machines in the same order they were written in. \
<p>\
Pass the result of serialize() to the Network Connection to send all values \
buffered up to this point. \
@param n The value to write \
 */ \
void write(T n); \
/** \
Write a vector of values. \
<p>\
Values will be deserialized on other machines in the same order they were written in. \
<p>\
Pass the result of serialize() to the Network Connection to send all values \
buffered up to this point. \
@param v The vector to write \
 */ \
void write(std::vector<T> v);

		WRITE_METHODS(bool, b);
		WRITE_METHODS(float, f);
		WRITE_METHODS(double, d);
		WRITE_METHODS(uint32_t, i);
		WRITE_METHODS(uint64_t, i);
		WRITE_METHODS(int32_t, i);
		WRITE_METHODS(int64_t, i);
		WRITE_METHODS(std::string, s);
		WRITE_METHODS(char*, s);
		WRITE_METHODS(std::shared_ptr<JsonValue>, j);

#undef WRITE_METHODS

		/**
		 * Serialize written values into a byte vector, suitable for network transit
		 * and subsequent deserialization.
		 * 
		 * You MUST call reset() after this method to clear the input buffer. Otherwise,
		 * the next call to this method will still contain all the contents written in this call.
		 * 
		 * Contains all values written to this object via calls to write(...) since the
		 * last call to this method.
		 * 
		 * The contents of the returned vector should be treated as opaque. Only
		 * read the output via use of the CUNetworkDeserializer class.
		 * 
		 * @returns A byte vector that can be deserialized with CUNetworkDeserializer.
		 */
		const std::vector<uint8_t>& serialize();

		/**
		 * Clear the input buffer.
		 */
		void reset();
	private:
		/** Buffer of data that has not been written out yet. */
		std::vector<uint8_t> data;
	};

	/**
	 * Helper class that deserializes byte arrays back into the original complex data.
	 *
	 * Intended for use with cugl::NetworkConnection.
	 *
	 * Only handles messages serialized using NetworkSerializer.
	 */
	class NetworkDeserializer {
	public:
		/**
		 * Variant of possible messages to receive.
		 * 
		 * Monostate represents no more content.
		 */
		typedef std::variant<
			std::monostate,
			bool,
			float,
			double,
			uint32_t,
			uint64_t,
			int32_t,
			int64_t,
			std::string,
			std::shared_ptr<JsonValue>,
			std::vector<bool>,
			std::vector<float>,
			std::vector<double>,
			std::vector<uint32_t>,
			std::vector<uint64_t>,
			std::vector<int32_t>,
			std::vector<int64_t>,
			std::vector<std::string>,
			std::vector<std::shared_ptr<JsonValue>>
		> Message;

		/**
		 * Load a new message to read.
		 * 
		 * Calling this method will discard any previously loaded messages.
		 * The message must be serialized by NetworkSerializer.
		 * 
		 * Once loaded, call read() to get a single value (or vector of values).
		 * The values are guaranteed to be delievered in the same order they were written.
		 * 
		 * The most common use case is to pass received byte vectors from the Network
		 * Connection to this method and then to call read() until it returns the monostate.
		 * 
		 * @param msg The byte vector serialized by NetworkSerializer
		 */
		void receive(const std::vector<uint8_t>& msg);

		/**
		 * Read the next unreturned value or vector from the currently loaded byte vector.
		 * 
		 * Load byte vectors with the receive(...) method.
		 * 
		 * If the end of the vector is reached, returns monostate.
		 * If nothing is loaded, this returns the monostate.
		 * 
		 * The return type is a variant. You can pattern match on the variant to handle
		 * different types. However, if you know what order the values were written in
		 * (which you really should), you can use std::get<T>(...) to just assert the next
		 * value should be of a certain type T and to extract that value directly. This avoids
		 * the overhead of a pattern match on every value.
		 */
		Message read();

		/**
		 * Clear the buffer and ignore any remaining data in it.
		 */
		void reset();
	private:
		/** Currently loaded data */
		std::vector<uint8_t> data;
		/** Position in the data of next byte to read */
		size_t pos = 0;
	};
}

#endif // CU_NETWORK_SERIALIZER_H
