#include <cugl/net/CUNetworkSerializer.h>
#include <cugl/base/CUEndian.h>

#include <stdexcept>
#include <sstream>

enum DataType : uint8_t {
	// Represents null in jsons
	None,
	BooleanTrue,
	BooleanFalse,
	Float,
	Double,
	UInt32,
	Int32,
	UInt64,
	Int64,
	String,
	Json,
	// Add the type stored in the array to this value
	// Use BooleanTrue to represent bool
	Array = 127
};

void cugl::NetworkSerializer::write(bool b) {
	data.push_back(b ? BooleanTrue : BooleanFalse);
}

void cugl::NetworkSerializer::write(std::string s) {
	data.push_back(String);
	write(s.size());
	for (char& c : s) {
		data.push_back(static_cast<uint8_t>(c));
	}
}

/**
 * Method to write a vector to the stream
 * 
 * @param T Type of the vector
 * @param TYPE Enum of the type stored inside the vector
 */
#define WRITE_VEC(T, TYPE) \
void cugl::NetworkSerializer::write(std::vector<T> v) {\
	data.push_back(Array + TYPE); \
	write(v.size()); \
	for (size_t i = 0; i < v.size(); i++) {	\
			write(v[i]); \
	}\
}

/**
 * Method to write a value and vectors of that value to the stream
 *
 * @param T Type of the value; must be numeric
 * @param TYPE Enum of the type
 */
#define WRITE_NUMERIC_METHODS(T, TYPE) \
void cugl::NetworkSerializer::write(T v) {\
	T ii = cugl::marshall(v);\
	const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&ii);\
	data.push_back(TYPE);\
	for (size_t j = 0; j < sizeof(T); j++) {\
		data.push_back(bytes[j]);\
	}\
}\
WRITE_VEC(T, TYPE)

WRITE_VEC(bool, BooleanTrue)
WRITE_NUMERIC_METHODS(float, Float)
WRITE_NUMERIC_METHODS(double, Double)
WRITE_NUMERIC_METHODS(uint32_t, UInt32)
WRITE_NUMERIC_METHODS(uint64_t, UInt64)
WRITE_NUMERIC_METHODS(int32_t, Int32)
WRITE_NUMERIC_METHODS(int64_t, Int64)
WRITE_VEC(char*, String)
WRITE_VEC(std::string, String)

void cugl::NetworkSerializer::write(char* v) {
	write(std::string(v));
}

void cugl::NetworkSerializer::write(std::shared_ptr<cugl::JsonValue> j) {
	data.push_back(Json);
	switch (j->type()) {
	case cugl::JsonValue::Type::NullType: 
		data.push_back(None);
		break;
	case cugl::JsonValue::Type::BoolType:
		write(j->asBool());
		break;
	case cugl::JsonValue::Type::NumberType:
		write(j->asDouble());
		break;
	case cugl::JsonValue::Type::StringType:
		write(j->asString());
		break;
	case cugl::JsonValue::Type::ArrayType: {
		data.push_back(Array);
		write(j->_children.size());
		for (auto& item : j->_children) {
			write(item);
		}
		break;
	}
	case cugl::JsonValue::Type::ObjectType:
		data.push_back(Json);
		write(j->_children.size());
		for (auto& item : j->_children) {
			write(item->key());
			write(item);
		}
		break;
	}
}
WRITE_VEC(std::shared_ptr<cugl::JsonValue>, Json)

const std::vector<uint8_t>& cugl::NetworkSerializer::serialize() {
	return data;
}

void cugl::NetworkSerializer::reset() {
	data.clear();
}


void cugl::NetworkDeserializer::receive(const std::vector<uint8_t>& msg) {
	data = msg;
	pos = 0;
}

#define DECODE_VEC(T, NAME) \
case Array + NAME: { \
	pos++; \
	size_t size = std::get<size_t>(read()); \
	std::vector<T> vv; \
	for (size_t i = 0; i < size; i++) { \
		vv.push_back(std::get<T>(read())); \
	} \
	return vv; \
}

#define DECODE_NUMERIC(T, NAME) \
case NAME:{\
	pos++;\
	const T* r = reinterpret_cast<const T*>(data.data() + pos);\
	pos += sizeof(T);\
	return cugl::marshall(*r);\
}\
DECODE_VEC(T, NAME)

cugl::NetworkDeserializer::Message cugl::NetworkDeserializer::read() {
	if (pos >= data.size()) {
		return {};
	}

	switch (data[pos]) {
	case None:
		pos++;
		return {};
	case BooleanTrue:
		pos++;
		return true;
	case BooleanFalse:
		pos++;
		return false;
	DECODE_NUMERIC(float, Float)
	DECODE_NUMERIC(double, Double)
	DECODE_NUMERIC(uint32_t, UInt32)
	DECODE_NUMERIC(uint64_t, UInt64)
	DECODE_NUMERIC(int32_t, Int32)
	DECODE_NUMERIC(int64_t, Int64)
	case String: {
		pos++;
		size_t size = std::get<size_t>(read());
		std::ostringstream disp;
		for (size_t i = 0; i < size; i++, pos++) {	
			disp << static_cast<char>(data[pos]);
		}
		return disp.str();
	}
	DECODE_VEC(bool, BooleanTrue)
	DECODE_VEC(std::string, String)
	case Json: {
		pos++;
		switch (data[pos]) {
		case None:
			pos++;
			return cugl::JsonValue::allocNull();
		case BooleanTrue:
			pos++;
			return cugl::JsonValue::alloc(true);
		case BooleanFalse:
			pos++;
			return cugl::JsonValue::alloc(false);
		case Double:
			return cugl::JsonValue::alloc(std::get<double>(read()));
		case String:
			return cugl::JsonValue::alloc(std::get<std::string>(read()));
		case Array: {
			auto ret = cugl::JsonValue::allocArray();
			pos++;
			size_t size = std::get<size_t>(read());
			for (size_t ii = 0; ii < size; ii++) {
				ret->appendChild(std::get<std::shared_ptr<cugl::JsonValue>>(read()));
			}
			return ret;
		}
		case Json: {
			auto ret = cugl::JsonValue::allocObject();
			pos++;
			size_t size = std::get<size_t>(read());
			for (size_t ii = 0; ii < size; ii++) {
				std::string key = std::get<std::string>(read());
				ret->appendChild(key, std::get<std::shared_ptr<cugl::JsonValue>>(read()));
			}
			return ret;
		}
		default:
			throw std::domain_error("Illegal json");
		}
	}
	DECODE_VEC(std::shared_ptr<cugl::JsonValue>, Json)
	default:
		throw std::domain_error("Illegal state of array; did you pass in a valid message?");
	}
}

void cugl::NetworkDeserializer::reset() {
	pos = 0;
	data.clear();
}
