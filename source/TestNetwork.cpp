#include "TestNetwork.h"

#include <cugl/cugl.h>

TestNetwork::TestNetwork() {
	net = std::make_shared<cugl::CUNetworkConnection>(
		cugl::CUNetworkConnection::ConnectionConfig(
			"34.74.68.73",
			61111,
			2,
			0)
		);
	cugl::Input::activate<cugl::Keyboard>();
}

void TestNetwork::step() {
	auto* k = cugl::Input::get<cugl::Keyboard>();
	if (k->keyPressed(cugl::KeyCode::SPACE)) {
		CULog("Sending");
		std::vector<uint8_t> msg = {1,2,3,4};
		net->send(msg);
	}
	net->receive([&](const std::vector<uint8_t> msg) {
		CULog("Received message of length %lu", msg.size());
		for (const auto& d : msg) {
			CULog("%d", d);
		}
		});
}
