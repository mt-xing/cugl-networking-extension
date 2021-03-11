#include "TestNetwork.h"

#include <cugl/cugl.h>

TestNetwork::TestNetwork() {
	net = std::make_shared<cugl::CUNetworkConnection>(cugl::CUNetworkConnection::ConnectionConfig("35.231.212.113", 61111, 2, 0));
}

void TestNetwork::step() {
	net->receive([&](const std::vector<uint8_t> msg) {
		CULog("Received message of length %lu", msg.size());
		for (const auto& d : msg) {
			CULog("%d", d);
		}
		});
}
