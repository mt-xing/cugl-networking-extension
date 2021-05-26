//
//  NetworkApp.h
//  Cornell University Game Library (CUGL)
//
//  This is the header for the custom application.  It is necessary so that
//  main.cpp can access your custom class.
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
#ifndef NETWORK_APP_H
#define NETWORK_APP_H
#include <cugl/cugl.h>

/**
 * Class for a simple networked game.
 *
 * The "game" consists of a single button players click to make a counter go up.
 * Each player's counter is visible on the screen of all other players.
 */
class NetworkApp : public cugl::Application {
private:
	/** Maximum number of players supported */
	constexpr static uint8_t NUM_PLAYERS = 3;

	/** Pointer to network connection object */
	std::shared_ptr<cugl::NetworkConnection> net;
	/** Helper class to serialize non-trivial network messages */
	cugl::NetworkSerializer serializer;
	/** Helper class to deserialize network message encoded with serializer */
	cugl::NetworkDeserializer deserializer;

#pragma region Scene Graph Nodes
#pragma mark Scene Graph Nodes

	/** A scene graph, used to display our 2D scenes */
	std::shared_ptr<cugl::Scene2> _scene;
	/** A 3152 style SpriteBatch to render the scene */
	std::shared_ptr<cugl::SpriteBatch>  _batch;

	// Subgraphs

	/** Subgraph for things shown on startup */
	std::shared_ptr<cugl::scene2::SceneNode> view1Start;
	/** Subgraph for things shown to host during matchmaking */
	std::shared_ptr<cugl::scene2::SceneNode> view2Host;
	/** Subgraph for things shown to client during matchmaking */
	std::shared_ptr<cugl::scene2::SceneNode> view3Client;
	/** Subgraph for things shown during gameplay */
	std::shared_ptr<cugl::scene2::SceneNode> view4Game;

	/** Font for labels (this demo uses Montserrat, licensed under the OFL) */
	std::shared_ptr<cugl::Font> font;

	// Individual scene graph nodes we keep a pointer to because we wish to update
	// their state during the course of the game

	/** Label on host screen */
	std::shared_ptr<cugl::scene2::Label> hostInfo;
	/** Label on client screen */
	std::shared_ptr<cugl::scene2::Label> clientInfo;
	/** Text input on client screen; they enter room IDs here */
	std::shared_ptr<cugl::scene2::TextField> clientInput;

	/** Label on game screen */
	std::shared_ptr<cugl::scene2::Label> gameInfo;
	/** Array of labels for each player's score on the game screen */
	std::array<std::shared_ptr<cugl::scene2::Label>, NUM_PLAYERS> gamePlayers;

#pragma mark -
#pragma endregion

	/** Scores for each player */
	std::array<double, NUM_PLAYERS> playerScores;

	/**
	 * Internal helper to build the scene graph.
	 */
	void buildScene();

public:
	/**
	 * Creates, but does not initialized a new application.
	 *
	 * This constructor is called by main.cpp.  You will notice that, like
	 * most of the classes in CUGL, we do not do any initialization in the
	 * constructor.  That is the purpose of the init() method.  Separation
	 * of initialization from the constructor allows main.cpp to perform
	 * advanced configuration of the application before it starts.
	 */
	NetworkApp() : Application(), playerScores() {}

	/**
	 * Disposes of this application, releasing all resources.
	 *
	 * This destructor is called by main.cpp when the application quits.
	 * It simply calls the dispose() method in Application.  There is nothing
	 * special to do here.
	 */
	~NetworkApp() { }

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
	virtual void onStartup() override;

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
	virtual void onShutdown() override;

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
	virtual void update(float timestep) override;

	/**
	 * The method called to draw the application to the screen.
	 *
	 * This is your core loop and should be replaced with your custom implementation.
	 * This method should OpenGL and related drawing calls.
	 *
	 * When overriding this method, you do not need to call the parent method
	 * at all. The default implmentation does nothing.
	 */
	virtual void draw() override;

};

#endif /* NETWORK_APP_H */
