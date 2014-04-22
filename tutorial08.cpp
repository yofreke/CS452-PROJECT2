// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "shader.hpp"
#include "texture.hpp"
#include "controls.hpp"
#include "objloader.hpp"
#include "vboindexer.hpp"
#include "Chest.cpp"

#include "maze.hpp"


struct SpawnedChest {
	float rotation;
	glm::vec3 position;
};

int mapCoord(int mapSize, glm::vec3 pos){
	return (int) (pos.x / 2) + (int) (pos.z / 2) * mapSize;
}

int findChest(std::vector<SpawnedChest> chests, glm::vec3 pos){
	for (int i = 0; i < chests.size(); ++i) {
		float dx = chests[i].position.x - pos.x;
		float dz = chests[i].position.z - pos.z;
		if(dx * dx + dz * dz < 2) {
			return i;
		}
	}
	return -1;
}


std::vector<glm::vec3> cube_vertices;
std::vector<glm::vec2> cube_uvs;
std::vector<glm::vec3> cube_normals;

std::vector<glm::vec3> map_vertices;
std::vector<glm::vec2> map_uvs;
std::vector<glm::vec3> map_normals;

int mapSize = 6;
int* mapArray;

std::vector<SpawnedChest> spawnedChests;

GLuint vertexbuffer;
GLuint uvbuffer;
GLuint normalbuffer;


void resetMap(){	
	mapArray = makeMaze(mapSize, mapSize);
	bool hasSetSpawn = false;

	map_vertices.clear();
	map_uvs.clear();
	map_normals.clear();
	spawnedChests.clear();

	for(int y = 0; y < mapSize; ++y){
		for(int x = 0; x < mapSize; ++x){
			int index = y * mapSize + x;
			if(mapArray[index] > 0){
				if(!hasSetSpawn){
					setCamPos(glm::vec3(x * 2 + 1, 1, y * 2 + 1));
				}

				if(spawnedChests.size() < 600 && (rand() % 10 < 2 || spawnedChests.size() == 0)){
					mapArray[index] = 2;
					SpawnedChest sc = SpawnedChest();
					sc.rotation = (float) (rand() % 314159) / 100000;

					sc.position.x = x * 2 + 1;
					sc.position.y = -0.95f;
					sc.position.z = y * 2 + 1;
					spawnedChests.push_back(sc);
				}

				bool hasNeighbor[] = {
					(y > 0 && mapArray[index - mapSize]),
					(x > 0 && mapArray[index - 1]),
					(y < mapSize - 1 && mapArray[index + mapSize]),
					(x < mapSize - 1 && mapArray[index + 1])
				};
				for(int faceIndex = 0; faceIndex < 6; ++faceIndex){
					if(faceIndex < 4 && hasNeighbor[faceIndex]) continue;

					for(int l = 0; l < 6; ++l){
						int mapIndex = map_vertices.size();
						map_vertices.push_back(cube_vertices[faceIndex * 6 + l]);
						map_vertices[mapIndex] += glm::vec3(x * 2 + 1, 0, y * 2 + 1);
						map_uvs.push_back(cube_uvs[faceIndex * 6 + l]);
						map_normals.push_back(cube_normals[faceIndex * 6 + l]);
					}
				}
			}
		}
	}

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, map_vertices.size() * sizeof(glm::vec3), &map_vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, map_uvs.size() * sizeof(glm::vec2), &map_uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, map_normals.size() * sizeof(glm::vec3), &map_normals[0], GL_STATIC_DRAW);
}

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);


	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Tutorial 08 - Basic Shading", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetCursorPos(window, 1024/2, 768/2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glDisable(GL_CULL_FACE);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "StandardShading.vertexshader", "StandardShading.fragmentshader" );

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	// Get a handle for our buffers
	GLuint vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");
	GLuint vertexUVID = glGetAttribLocation(programID, "vertexUV");
	GLuint vertexNormal_modelspaceID = glGetAttribLocation(programID, "vertexNormal_modelspace");

	// Load the texture
	// GLuint Texture = loadDDS("uvmap.DDS");
	GLuint Texture = loadBMP_custom("floor.bmp");
	
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	bool res = loadOBJ("cube_floor.obj", cube_vertices, cube_uvs, cube_normals);

	// Load it into a VBO
	resetMap();

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	// GLuint LightID2 = glGetUniformLocation(programID, "LightPosition2_worldspace");

	loadChest();


	float rotX = 0.f;

	do{

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs(mapArray, mapSize);

		// Check for treasure!
		int foundChestId = findChest(spawnedChests, getCamPos());
		if(foundChestId >= 0){
			// printf("FOUND CHEST ID %d\n", foundChestId);
			spawnedChests.erase(spawnedChests.begin() + foundChestId);
			if(spawnedChests.size() > 0)
				printf("You have scored!  Only %d more chests to find!\n", (int) spawnedChests.size());
			else {
				mapSize += 2;
				printf("You have cleared this phase, new map size of %dx%d!!!\n", mapSize, mapSize);
				resetMap();
			}
		}

		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		// glm::mat4 ProjectionMatrix = glm::mat4(1);
		// glm::mat4 ViewMatrix = glm::lookAt(
		// 						glm::vec3(0),           // Camera is here
		// 						chestPos, // and looks here : at the same position, plus "direction"
		// 						glm::vec3(0,1,0)                  // Head is up (set to 0,-1,0 to look upside-down)
		// 				   );
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1);

		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		// glm::vec3 lightPos = glm::vec3(-15,-10,0);
		glm::vec3 lightPos = getCamPos();
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		// glm::vec3 lightPos2 = glm::vec3(0,0,10);
		// glUniform3f(LightID2, lightPos2.x, lightPos2.y, lightPos2.z);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(vertexPosition_modelspaceID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			vertexPosition_modelspaceID,  // The attribute we want to configure
			3,                            // size
			GL_FLOAT,                     // type
			GL_FALSE,                     // normalized?
			0,                            // stride
			(void*)0                      // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(vertexUVID);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			vertexUVID,                   // The attribute we want to configure
			2,                            // size : U+V => 2
			GL_FLOAT,                     // type
			GL_FALSE,                     // normalized?
			0,                            // stride
			(void*)0                      // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(vertexNormal_modelspaceID);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			vertexNormal_modelspaceID,    // The attribute we want to configure
			3,                            // size
			GL_FLOAT,                     // type
			GL_FALSE,                     // normalized?
			0,                            // stride
			(void*)0                      // array buffer offset
		);

		// Draw the triangles !
		glDrawArrays(GL_TRIANGLES, 0, map_vertices.size() );
		// glDrawArrays(GL_TRIANGLES, 0, ChestNumVerts );

		///////////////////////////////////////////////
		for (std::vector<SpawnedChest>::iterator i = spawnedChests.begin(); i != spawnedChests.end(); ++i)
		{

			ModelMatrix = getChestMatrix(&(*i).rotation, (*i).position);
			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

			// Send our transformation to the currently bound shader, 
			// in the "MVP" uniform
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
			glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

			renderChest(programID);	
		}
		//////////////////////////////////////////////

		glDisableVertexAttribArray(vertexPosition_modelspaceID);
		glDisableVertexAttribArray(vertexUVID);
		glDisableVertexAttribArray(vertexNormal_modelspaceID);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

