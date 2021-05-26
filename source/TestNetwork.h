#ifndef TEST_NETWORK_H
#define TEST_NETWORK_H

#include <cugl/cugl.h>

class TestNetwork {
private:
	std::shared_ptr<cugl::NetworkConnection> net;
public:
	TestNetwork();

	void step();
};

#endif // TEST_NETWORK_H