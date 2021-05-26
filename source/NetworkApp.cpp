//
//  NetworkApp.cpp
//  Cornell University Game Library (CUGL)
//
//  This is the implementation file for the custom application. This is the
//  definition of your root (and in this case only) class.
//
//  CUGL zlib License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Author: Michael Xing
//  Version: 5/25/2021
//
// Include the class header, which includes all of the CUGL classes
#include "NetworkApp.h"
#include <cugl/base/CUBase.h>

#include <cstdio>

using namespace cugl;
// This is adjusted by screen aspect ratio to get the height
#define GAME_WIDTH 1024

// Network connection configuration, with punchthrough server IP,
// server port, num players, and api version
#define NETWORK_CONFIG cugl::NetworkConnection::ConnectionConfig(\
	"34.74.68.73",\
	61111,\
	3,\
	0\
)

/**
 * The method called after OpenGL is initialized, but before running the application.
 *
 * This is the method in which all user-defined program intialization should
 * take place.  You should not create a new init() method.
 *
 * When overriding this method, you should call the parent method as the
 * very last line.  This ensures that the state will transition to FOREGROUND,
 * causing the application to run.
 */
void NetworkApp::onStartup() {
	cugl::Size size = getDisplaySize();
	size *= GAME_WIDTH / size.width;

	// Create a scene graph the same size as the window
	_scene = Scene2::alloc(size.width, size.height);
	// Create a sprite batch (and background color) to render the scene
	_batch = SpriteBatch::alloc();
	setClearColor(Color4(229, 229, 229, 255));

	// In a larger app you'd probably want the matchmaking phase to be a whole separate
	// scene from the main game itself, but for simplicity here we'll just make each thing
	// a separate node and show / hide them as needed.
	view1Start = scene2::SceneNode::allocWithBounds(0, 0, size.width, size.height);
	view2Host = scene2::SceneNode::allocWithBounds(0, 0, size.width, size.height);
	view3Client = scene2::SceneNode::allocWithBounds(0, 0, size.width, size.height);
	view4Game = scene2::SceneNode::allocWithBounds(0, 0, size.width, size.height);

	// Initialize the font (you can and probably should read this out of a json in a
	// larger game project).
	font = Font::alloc("montserrat.ttf", 16);

	// Activate mouse or touch screen input as appropriate
	// We have to do this BEFORE the scene, because the scene has a button
#if defined (CU_TOUCH_SCREEN)
	Input::activate<Touchscreen>();
#else
	Input::activate<Mouse>();
	Input::activate<Keyboard>();
#endif
	Input::activate<TextInput>();

	// Build the scene from these assets
	buildScene();
	Application::onStartup();
}

/**
 * The method called when the application is ready to quit.
 *
 * This is the method to dispose of all resources allocated by this
 * application.  As a rule of thumb, everything created in onStartup()
 * should be deleted here.
 *
 * When overriding this method, you should call the parent method as the
 * very last line.  This ensures that the state will transition to NONE,
 * causing the application to be deleted.
 */
void NetworkApp::onShutdown() {
	// Delete all smart pointers
	_scene = nullptr;
	_batch = nullptr;
	view1Start = nullptr;
	view2Host = nullptr;
	view3Client = nullptr;
	view4Game = nullptr;
	font = nullptr;
	hostInfo = nullptr;
	clientInfo = nullptr;
	clientInput = nullptr;
	gameInfo = nullptr;
	for (size_t i = 0; i < NUM_PLAYERS; i++) {
		gamePlayers[i] = nullptr;
	}

	// Deativate input
#if defined CU_TOUCH_SCREEN
	Input::deactivate<Touchscreen>();
#else
	Input::deactivate<Mouse>();
	Input::deactivate<Keyboard>();
#endif
	Input::deactivate<TextInput>();
	Application::onShutdown();
}

void NetworkApp::buildScene() {

	// For simplicity, this demo predominately uses buttons via callbacks instead of
	// polling them every frame. This means a lot of the functionality of the game
	// is actually registered in this method.

	// Labels are initialized with dummy text that are longer than their actual text
	// will be; otherwise, the label will clip your new text (their width is fixed).

	// Setup Start Phase
	auto hostBtn = scene2::Button::alloc(scene2::Label::alloc("Host New Game", font));
	hostBtn->setPosition(100, 200);
	hostBtn->addListener([&](const std::string& name, bool down) {
		if (down && view1Start->isVisible()) {
			CULog("Clicked host");
			view1Start->setVisible(false);
			view2Host->setVisible(true);
			// Initialize a network connection as a host - just pass it a network config.
			// This is NOT instant; when the connection status changed to CONNECTED, then
			// getRoomID() will have your assigned room ID.
			// Make sure you call receive() every frame until the game starts.
			net = std::make_shared<NetworkConnection>(NETWORK_CONFIG);
		}
		});
	hostBtn->activate();
	view1Start->addChild(hostBtn);
	auto clientBtn = scene2::Button::alloc(scene2::Label::alloc("Join Existing Game", font));
	clientBtn->setPosition(100, 100);
	clientBtn->addListener([&](const std::string& name, bool down) {
		if (down && view1Start->isVisible()) {
			CULog("Clicked client");
			view1Start->setVisible(false);
			view3Client->setVisible(true);
		}
		});
	clientBtn->activate();
	view1Start->addChild(clientBtn);

	// Setup Host Phase
	hostInfo = scene2::Label::alloc("Room ID: unassigned, # of Players: unknown", font);
	hostInfo->setPosition(100, 200);
	view2Host->addChild(hostInfo);
	auto hostStartBtn = scene2::Button::alloc(scene2::Label::alloc("Start Game", font));
	hostStartBtn->setPosition(100, 100);
	hostStartBtn->addListener([&](const std::string& name, bool down) {
		if (down && view2Host->isVisible() && net->getStatus() == NetworkConnection::NetStatus::Connected) {
			CULog("Clicked start");
			view2Host->setVisible(false);
			view4Game->setVisible(true);
			// You can send byte vectors like this directly
			// Or you can choose to use NetworkSerializer as shown down in update()
			net->send({ 0 });
			net->startGame();
			std::ostringstream disp;
			disp << "You are player ";
			disp << static_cast<int>(*net->getPlayerID());
			gameInfo->setText(disp.str());
		}
		});
	hostStartBtn->activate();
	view2Host->addChild(hostStartBtn);

	// Setup Client Phase
	clientInfo = scene2::Label::alloc("Enter Room ID Below", font);
	clientInfo->setPosition(100, 300);
	view3Client->addChild(clientInfo);
	clientInput = scene2::TextField::alloc("00000", font);
	clientInput->setPosition(100, 200);
	clientInput->activate();
	view3Client->addChild(clientInput);
	auto clientStartBtn = scene2::Button::alloc(scene2::Label::alloc("Join", font));
	clientStartBtn->setPosition(100, 100);
	clientStartBtn->addListener([&](const std::string& name, bool down) {
		if (down && view3Client->isVisible() && net == nullptr) {
			CULog("Clicked join");
			clientInfo->setText("Connecting...");
			// Initialize a network connection as a client -
			// pass it a network config and a room ID as a string.
			// This is NOT instant; when the connection status changed to CONNECTED, then
			// getPlayerID() will have your assigned player ID.
			// Player ID 0 is reserved for the host.
			// Make sure you call receive() every frame until the game starts.
			net = std::make_shared<NetworkConnection>(NETWORK_CONFIG, clientInput->getText());
		}
		});
	clientStartBtn->activate();
	view3Client->addChild(clientStartBtn);
	
	// Setup Game Phase
	auto gameTitle = scene2::Label::alloc("Extreme Cookie Clicker Multiplayer Edition", font);
	gameTitle->setPosition(100, 350);
	view4Game->addChild(gameTitle);
	auto gameDesc = scene2::Label::alloc("Click the cookie to make the number go up. That's it. That's the game.", font);
	gameDesc->setPosition(100, 300);
	view4Game->addChild(gameDesc);
	auto gameBtn = scene2::Button::alloc(scene2::Label::alloc("THE COOKIE (click me)", font));
	gameBtn->setPosition(100, 250);
	gameBtn->addListener([&](const std::string& name, bool down) {
		if (down && view4Game->isVisible()) {
			CULog("Clicked cookie");
			// getPlayerID() returns an optional
			// I'm dereferencing it directly because the class invariant should guarantee
			// that if view4Game is visible, then we're in the game proper and thus have a player ID.
			playerScores[*net->getPlayerID()]++;

			// Demonstrating using network serialization to send a message instead.
			// Serializers must be manually reset before each use.
			// (This allows you to call serialize() multiple times for the same message
			// if you ever want to for some reason)
			serializer.reset();
			JsonValue msg;
			msg.initObject();
			msg.appendValue("player", static_cast<double>(*net->getPlayerID()));
			msg.appendValue("amount", static_cast<double>(playerScores[*net->getPlayerID()]));
			// Serializer can write booleans, numeric types, strings, and JSONs.
			// They'll be read off on the other end in the same order they were written in.
			serializer.write(std::make_shared<JsonValue>(msg));
			net->send(serializer.serialize());
		}
		});
	gameBtn->activate();
	view4Game->addChild(gameBtn);
	gameInfo = scene2::Label::alloc("You are player number <unknown>", font);
	gameInfo->setPosition(100, 200);
	view4Game->addChild(gameInfo);
	for (size_t i = 0; i < NUM_PLAYERS; i++) {
		gamePlayers[i] = scene2::Label::alloc("Player number clicked <unknown> times", font);
		gamePlayers[i]->setPosition(100, 150 - i * 50);
		view4Game->addChild(gamePlayers[i]);
	}

	for (size_t i = 0; i < NUM_PLAYERS; i++) {
		playerScores[i] = 0;
	}


	// Add everything to the scene
	view1Start->setVisible(true);
	view2Host->setVisible(false);
	view3Client->setVisible(false);
	view4Game->setVisible(false);
	_scene->addChild(view1Start);
	_scene->addChild(view2Host);
	_scene->addChild(view3Client);
	_scene->addChild(view4Game);
}

/**
 * The method called to update the application data.
 *
 * This is your core loop and should be replaced with your custom implementation.
 * This method should contain any code that is not an OpenGL call.
 *
 * When overriding this method, you do not need to call the parent method
 * at all. The default implmentation does nothing.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void NetworkApp::update(float timestep) {
	if (net == nullptr) {
		// Phase 1 or 3
		// There's nothing we need to do in either of these phases if net is null.
		return;
	}
	else {
		switch (net->getStatus()) {
		case NetworkConnection::NetStatus::Disconnected:
			break;
		case NetworkConnection::NetStatus::Pending:
			break;
		case NetworkConnection::NetStatus::Connected:

			// It might not be super smart to change the text every frame.
			// But for this demo I don't care.
			if (view2Host->isVisible()) {
				std::ostringstream disp;
				disp << "Room ID: ";
				disp << net->getRoomID();
				disp << " # of Players: ";
				disp << static_cast<int>(net->getNumPlayers());
				hostInfo->setText(disp.str());
			}
			else if (view3Client->isVisible()) {
				std::ostringstream disp;
				disp << "Connected to ";
				disp << static_cast<int>(net->getNumPlayers());
				disp << " players";
				clientInfo->setText(disp.str());
				clientInput->deactivate();
			}
			else if (view4Game->isVisible()) {
				for (uint8_t i = 0; i < NUM_PLAYERS; i++) {
					std::ostringstream disp;
					disp << "Player ";
					disp << static_cast<int>(i);
					disp << " clicked ";
					disp << playerScores[i];
					disp << " times";
					gamePlayers[i]->setText(disp.str());
				}
			}

			break;
		case NetworkConnection::NetStatus::Reconnecting:
			gameInfo->setText("Reconnecting");
			break;
		case NetworkConnection::NetStatus::RoomNotFound:
			clientInfo->setText("Invalid room");
			net = nullptr;
			return;
		case NetworkConnection::NetStatus::ApiMismatch:
			// For this demo, we're treating this error as unrecoverable
			clientInfo->setText("Outdated app");
			hostInfo->setText("Outdated app");
			net = nullptr;
			return;
		case NetworkConnection::NetStatus::GenericError:
			// For this demo, we're treating this error as unrecoverable
			clientInfo->setText("Unknown error");
			hostInfo->setText("Unknown error");
			net = nullptr;
			return;
		}

		// Call receive even when the connection has not been established yet.
		// This is the method that steps the library so it can do things like broker connections.
		// You don't have to call it every frame during gameplay, but during the matchmaking
		// phase, you should (the punchthrough library doesn't work reliably if this is not called
		// frequently enough).
		net->receive([this](auto msg) {
			// There's that one message we send as a raw byte vector when the game is starting.
			// In that case, msg is your byte vector, and you just read it like any other vector.
			if (msg.size() == 1 && msg[0] == 0 && view3Client->isVisible()) {
				view3Client->setVisible(false);
				view4Game->setVisible(true);

				std::ostringstream disp;
				disp << "You are player ";
				disp << static_cast<int>(*net->getPlayerID());
				gameInfo->setText(disp.str());
				return;
			}

			// All other messages we send are with network serializer, so we must
			// now deserialize it on the other end.
			deserializer.receive(msg);
			// read() returns a variant (Ocaml style) with all possible value types.
			// You can pattern match on the variant if you want, or if you know what
			// you wrote, you can use std::get<>() to extract the value directly.
			auto jsonMsg = std::get<std::shared_ptr<JsonValue>>(deserializer.read());
			uint8_t player = static_cast<uint8_t>(jsonMsg->getDouble("player"));
			double amount = jsonMsg->getDouble("amount");
			playerScores[player] = amount;
		});
	}
}

/**
 * The method called to draw the application to the screen.
 *
 * This is your core loop and should be replaced with your custom implementation.
 * This method should OpenGL and related drawing calls.
 *
 * When overriding this method, you do not need to call the parent method
 * at all. The default implmentation does nothing.
 */
void NetworkApp::draw() {
	// This takes care of begin/end
	_scene->render(_batch);
}
