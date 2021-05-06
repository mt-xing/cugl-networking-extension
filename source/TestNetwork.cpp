#include "TestNetwork.h"

#include <cugl/cugl.h>
#include <cugl/net/CUNetworkSerializer.h>

static uint8_t lastNum = 942;

TestNetwork::TestNetwork() {
	net = std::make_shared<cugl::CUNetworkConnection>(
		cugl::CUNetworkConnection::ConnectionConfig(
			"34.74.68.73",
			61111,
			3,
			0)
		);
	cugl::Input::activate<cugl::Keyboard>();

	cugl::CUNetworkSerializer test;
	test.write("hello world");
	test.write(-123.4);
	test.write((int64_t)5);
	std::vector<std::string> testV = { "hi" };
	test.write(testV);
	const auto& d = test.serialize();
	test.receive(d);
	CULog("String msg: %s", std::get<std::string>(test.read()).c_str());
	CULog("Float msg: %f", std::get<double>(test.read()));
	CULog("Int msg: %d", std::get<int64_t>(test.read()));
	CULog("String msg: %s", std::get<std::string>(test.read()).c_str());
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
