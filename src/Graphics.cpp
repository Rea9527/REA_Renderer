// Graphics.cpp : Defines the entry point for the console application.
//


#include <iostream>
#include <string>
#include <vector>
#include <memory>

using namespace std;

// GLFW
#include <glad/include/glad/glad.h>
#include <GLFW/glfw3.h>
// include user define files
//#include "SceneManager.h"
#include "scenes/SceneToon.h"
#include "scenes/SceneCloth.h"
#include "scenes/SceneSPH.h"
#include "scenes/SceneBloom.h"
#include "scenes/SceneShadowMap.h"
#include "scenes/SceneTerrain.h"
#include "scenes/SceneDefer.h"
#include "scenes/SceneSSAO.h"
#include "scenes/ScenePBR.h"

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

// The MAIN function, from here we start the application and run the game loop
int main()
{	
	//=========================================================================================================================================
	// Initialization
	//=========================================================================================================================================
	SceneManager manager(WIDTH, HEIGHT, "GLSL");
	
	std::unique_ptr<Scene> scene = std::unique_ptr<Scene>(new ScenePBR(WIDTH, HEIGHT));
	//std::unique_ptr<Scene> scene = std::unique_ptr<Scene>(new SceneBloom(WIDTH, HEIGHT));
	//std::unique_ptr<Scene> scene = std::unique_ptr<Scene>(new SceneToon(WIDTH, HEIGHT));
	//std::unique_ptr<Scene> scene = std::unique_ptr<Scene>(new SceneCloth());
	//std::unique_ptr<Scene> scene = std::unique_ptr<Scene>(new SceneSPH());

	return manager.run(*scene);
	
	//=========================================================================================================================================
	// End of initialization
	//=========================================================================================================================================



	//=========================================================================================================================================
	// create shader program
	//=========================================================================================================================================
	
	//=========================================================================================================================================
	// End of creating program
	//=========================================================================================================================================


	//=========================================================================================================================================
	// Triangle setup
	//=========================================================================================================================================
	// Set up vertex data (and buffer(s)) and attribute pointers
	// We add a new set of vertices to form a second triangle (a total of 6 vertices); the vertex attribute configuration remains the same (still one 3-float position vector per vertex)
	
	//=========================================================================================================================================
	// End of triangle setup
	//=========================================================================================================================================


	//=========================================================================================================================================
	// Main loop
	//=========================================================================================================================================
	
}
//=========================================================================================================================================
// End of main loop
//=========================================================================================================================================

