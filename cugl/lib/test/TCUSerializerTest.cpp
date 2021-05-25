#include "TCUSerializerTest.h"

#include <cugl/cugl.h>
#include <cugl/net/CUNetworkSerializer.h>

void cugl::serializerUnitTest() {
	cugl::simpleTest();
	cugl::testNumericTypes();
	cugl::testStrings();
}

void cugl::simpleTest() {
	cugl::CUNetworkSerializer test;
	test.write("hello world");
	test.write(-123.4);
	test.write((int64_t)5);
	std::vector<std::string> testV = { "hi" };
	test.write(testV);

	std::vector<uint8_t> d(test.serialize());

	cugl::CUNetworkDeserializer test2;
	test2.receive(d);

	CUAssertAlwaysLog(std::get<std::string>(test2.read()) == "hello world", "string test");
	CUAssertAlwaysLog(std::get<double>(test2.read()) == -123.4, "double test");
	CUAssertAlwaysLog(std::get<int64_t>(test2.read()) == 5, "int test");
	auto vv = std::get<std::vector<std::string>>(test2.read());
	CUAssertAlwaysLog(vv.size() == 1, "vector test");
	CUAssertAlwaysLog(vv[0] == "hi", "vector test");
}

void cugl::testNumericTypes() {
	std::vector<uint32_t> u32 = {
		0,1,2,3,4,5,13092285,
		std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max()
	};
	std::vector<int32_t> s32 = {
		-1,0,1,2,3,4,5,10,234523423,std::numeric_limits<int32_t>::min(),std::numeric_limits<int32_t>::max()
	};
	std::vector<uint64_t> u64 = {
		0,1,2,3,4,5,13092285,std::numeric_limits<uint64_t>::min(),std::numeric_limits<uint64_t>::max()
	};
	std::vector<int64_t> s64 = {
		-1,0,1,2,3,4,5,10,234523423,std::numeric_limits<int64_t>::min(),std::numeric_limits<int64_t>::max()
	};
	std::vector<float> f = {
		-1,0,1,2,3,4,1.1f,0.1f,2324.23423f,-23422,4393,-34534.3453f,-0.000001f,
		std::numeric_limits<float>::min(),
		std::numeric_limits<float>::max(),
		std::numeric_limits<float>::lowest(),
		INFINITY, -INFINITY
	};
	std::vector<double> d = {
		-1,0,1,2,3,4,1.1,0.1,2324.23423,-23422,4393,-34534.3453,-0.0000001,
		std::numeric_limits<double>::min(),
		std::numeric_limits<double>::max(),
		std::numeric_limits<double>::lowest(),
		std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()
	};

	cugl::CUNetworkSerializer test;
	for (auto& e : u32) {
		test.write(e);
	}
	for (auto& e : s32) {
		test.write(e);
	}
	for (auto& e : u64) {
		test.write(e);
	}
	for (auto& e : s64) {
		test.write(e);
	}
	for (auto& e : f) {
		test.write(e);
	}
	for (auto& e : d) {
		test.write(e);
	}

	std::vector<uint8_t> dd(test.serialize());
	cugl::CUNetworkDeserializer test2;
	test2.receive(dd);
	for (auto& e : u32) {
		CUAssertAlwaysLog(e == std::get<uint32_t>(test2.read()), "uint32 test");
	}
	for (auto& e : s32) {
		CUAssertAlwaysLog(e == std::get<int32_t>(test2.read()), "int32 test");
	}
	for (auto& e : u64) {
		CUAssertAlwaysLog(e == std::get<uint64_t>(test2.read()), "uint64 test");
	}
	for (auto& e : s64) {
		CUAssertAlwaysLog(e == std::get<int64_t>(test2.read()), "int64 test");
	}
	for (auto& e : f) {
		CUAssertAlwaysLog(e == std::get<float>(test2.read()), "float test");
	}
	for (auto& e : d) {
		CUAssertAlwaysLog(e == std::get<double>(test2.read()), "double test");
	}
}

void cugl::testStrings() {
}
