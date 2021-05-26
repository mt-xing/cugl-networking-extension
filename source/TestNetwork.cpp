#include "TestNetwork.h"

#include <cugl/cugl.h>
// Super fragile include; here only for testing purposes
#include <cugl/../../lib/test/TCUSerializerTest.h>

static uint8_t lastNum = 942;
constexpr bool RUN_TESTS = true;

TestNetwork::TestNetwork() {
	net = std::make_shared<cugl::NetworkConnection>(
		cugl::NetworkConnection::ConnectionConfig(
			"34.74.68.73",
			61111,
			3,
			0)
		);
	cugl::Input::activate<cugl::Keyboard>();

	if (RUN_TESTS) {
		cugl::serializerUnitTest();
	}
}

void TestNetwork::step() {
	auto* k = cugl::Input::get<cugl::Keyboard>();
	
	if (k->keyPressed(cugl::KeyCode::SPACE)) {
		CULog("Sending");
		std::vector<uint8_t> msg = {1,2,3,4};
		net->send(msg);
	}

	if (k->keyPressed(cugl::KeyCode::S)) {
		net->startGame();
	}

	net->receive([&](const std::vector<uint8_t> msg) {
		CULog("Received message of length %lu", msg.size());
		for (const auto& d : msg) {
			CULog("%d", d);
		}
		});
	if (net->getNumPlayers() != lastNum) {
		lastNum = net->getNumPlayers();
		CULog("Num players %d, total players %d", net->getNumPlayers(), net->getTotalPlayers());
		CULog("Room ID %s", net->getRoomID().c_str());
		if (net->getPlayerID().has_value()) {
			CULog("Player ID %d", *net->getPlayerID());
		}
	}
}
