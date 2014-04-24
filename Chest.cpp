// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "shader.hpp"
#include "texture.hpp"
#include "objloader.hpp"
// #include "vboindexer.hpp"

#ifndef CHEST_CPP
#define CHEST_CPP

GLuint chestVertBuffer;
GLuint chestUVBuffer;
GLuint chestNormalBuffer;

std::vector<glm::vec3> chestVerts = std::vector<glm::vec3>();
std::vector<glm::vec2> chestUVS = std::vector<glm::vec2>();
std::vector<glm::vec3> chestNormals = std::vector<glm::vec3>();

GLuint chestTexture = -1;

float chestRotX = 0.0f;

void loadChest()
{
	// Create and compile our GLSL program from the shaders
	// GLuint programID = LoadShaders( "StandardShading.vertexshader", "StandardShading.fragmentshader" );

	// Load the texture
	// GLuint chestTexture = loadDDS("uvmap.DDS");
	chestTexture = loadBMP_custom("diffuse.bmp");
	
	// Get a handle for our "myTextureSampler" uniform
	// GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	bool res = loadOBJ("Chest_blend.obj", chestVerts, chestUVS, chestNormals);

	// Load it into a VBO

	glGenBuffers(1, &chestVertBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, chestVertBuffer);
	glBufferData(GL_ARRAY_BUFFER, chestVerts.size() * sizeof(glm::vec3), &chestVerts[0], GL_STATIC_DRAW);
	// glBufferData(GL_ARRAY_BUFFER, ChestNumVerts * sizeof(float), &ChestVerts[0], GL_STATIC_DRAW);
	
	glGenBuffers(1, &chestUVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, chestUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, chestUVS.size() * sizeof(glm::vec2), &chestUVS[0], GL_STATIC_DRAW);
	// glBufferData(GL_ARRAY_BUFFER, ChestNumVerts * sizeof(float), &ChestTexCoords[0], GL_STATIC_DRAW);
	
	glGenBuffers(1, &chestNormalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, chestNormalBuffer);
	glBufferData(GL_ARRAY_BUFFER, chestNormals.size() * sizeof(glm::vec3), &chestNormals[0], GL_STATIC_DRAW);
	// glBufferData(GL_ARRAY_BUFFER, ChestNumVerts * sizeof(float), &ChestNormals[0], GL_STATIC_DRAW);
}

glm::mat4 getChestMatrix(float* rotation, glm::vec3 position) {
	*rotation += 0.005f;
	if(*rotation > 6.283f) *rotation -= 6.283f;

	glm::mat4 ModelMatrix = glm::mat4(1);
	ModelMatrix=glm::translate(ModelMatrix, position);
	ModelMatrix=glm::rotate(ModelMatrix, 90.f * (*rotation), glm::vec3(0, 1, 0));
	ModelMatrix=glm::scale(ModelMatrix, glm::vec3(0.01f));

	return ModelMatrix;
}

void renderChest(GLuint programID) {
	

	// glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

	// glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform
	// glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	// glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
	// glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

	GLuint vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");
	GLuint vertexUVID = glGetAttribLocation(programID, "vertexUV");
	GLuint vertexNormal_modelspaceID = glGetAttribLocation(programID, "vertexNormal_modelspace");
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Bind our texture in chestTexture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, chestTexture);
	// Set our "myTextureSampler" sampler to user chestTexture Unit 0
	glUniform1i(TextureID, 0);

	// 1rst attribute buffer : chestVerts
	glEnableVertexAttribArray(vertexPosition_modelspaceID);
	glBindBuffer(GL_ARRAY_BUFFER, chestVertBuffer);
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
	glBindBuffer(GL_ARRAY_BUFFER, chestUVBuffer);
	glVertexAttribPointer(
		vertexUVID,                   // The attribute we want to configure
		2,                            // size : U+V => 2
		GL_FLOAT,                     // type
		GL_FALSE,                     // normalized?
		0,                            // stride
		(void*)0                      // array buffer offset
	);

	// 3rd attribute buffer : chestNormals
	glEnableVertexAttribArray(vertexNormal_modelspaceID);
	glBindBuffer(GL_ARRAY_BUFFER, chestNormalBuffer);
	glVertexAttribPointer(
		vertexNormal_modelspaceID,    // The attribute we want to configure
		3,                            // size
		GL_FLOAT,                     // type
		GL_FALSE,                     // normalized?
		0,                            // stride
		(void*)0                      // array buffer offset
	);

	// Draw the triangles !
	glDrawArrays(GL_TRIANGLES, 0, chestVerts.size() );
	// glDrawArrays(GL_TRIANGLES, 0, ChestNumVerts );

	glDisableVertexAttribArray(vertexPosition_modelspaceID);
	glDisableVertexAttribArray(vertexUVID);
	glDisableVertexAttribArray(vertexNormal_modelspaceID);


	// Cleanup VBO and shader
	// glDeleteBuffers(1, &chestVertBuffer);
	// glDeleteBuffers(1, &chestUVBuffer);
	// glDeleteBuffers(1, &chestNormalBuffer);
	// glDeleteProgram(programID);
	// glDeleteTextures(1, &chestTexture);
}

#endif