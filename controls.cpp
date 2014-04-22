// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

#include <stdio.h>

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}


// Initial position : on +Z
glm::vec3 cameraPosition = glm::vec3( 1, 1, 1 ); 
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 4.5f; // 3 units / second
float mouseSpeed = 0.005f;

float stepOffsetTimer = 0;
float stepOffset = 0;

float velY = 0.0f;
bool useGrav = true;

glm::vec3 getCamPos() { return cameraPosition; }
void setCamPos(glm::vec3 pos) { cameraPosition = pos; }

float collisionMargin = 0.25f;

extern int mapCoord(int mapSize, glm::vec3 pos);

int getMapIdAt(int* map, int mapSize, glm::vec3 pos){
	return map[mapCoord(mapSize, pos)];
}

bool isValid(int* map, int mapSize, glm::vec3 pos){
	if(pos.x < collisionMargin || pos.x > mapSize * 2 - collisionMargin
		|| pos.z < collisionMargin || pos.z > mapSize * 2 - collisionMargin) return false;

	int currentTile = getMapIdAt(map, mapSize, pos);
	if(currentTile == 0) return false;
	return true;
}

void computeMatricesFromInputs(int* map, int mapSize){

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// Reset mouse position for next frame
	if(!useGrav) glfwSetCursorPos(window, 1024/2, 768/2);

	// Compute new orientation
	if(!useGrav){
		horizontalAngle += mouseSpeed * float(1024/2 - xpos );
		verticalAngle   += mouseSpeed * float( 768/2 - ypos );
	}

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle), 
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);
	
	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f/2.0f), 
		0,
		cos(horizontalAngle - 3.14f/2.0f)
	);
	
	// Up vector
	glm::vec3 up = glm::cross( right, direction );

	bool movingFlag = false;
	glm::vec3 newPosition = glm::vec3(cameraPosition);
	if(useGrav){
		// Move forward
		if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
			newPosition += direction * deltaTime * speed;
			movingFlag = true;
		}
		// Move backward
		if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
			newPosition -= direction * deltaTime * speed;
			movingFlag = true;
		}
		// Strafe right
		if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
			horizontalAngle -= deltaTime * speed;
		}
		// Strafe left
		if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
			horizontalAngle += deltaTime * speed;
		}
	} else {
		// Move forward
		if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
			newPosition += direction * deltaTime * speed;
		}
		// Move backward
		if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
			newPosition -= direction * deltaTime * speed;
		}
		// Strafe right
		if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
			newPosition += right * deltaTime * speed;
		}
		// Strafe left
		if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
			newPosition -= right * deltaTime * speed;
		}
	}

	// Collisions and gravity //
	if(useGrav){
		velY -= 0.001f;
		newPosition += glm::vec3(0.0f, velY, 0.0f);

		if(newPosition.y < 0) {
			newPosition.y = 0;
			velY *= -0.35;
		}

		if(movingFlag){
			stepOffsetTimer += 0.15;
			stepOffset = ((float) sin(stepOffsetTimer) + 0.5f) * -0.075f;
		} else {
			stepOffset += (0 - stepOffset) * 0.05f;
			stepOffsetTimer = 0;
		}
	}
	// ... //

	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	if(!useGrav || isValid(map, mapSize, newPosition)){
		cameraPosition = newPosition;
	}

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(FoV, 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix       = glm::lookAt(
								cameraPosition,           // Camera is here
								cameraPosition+direction, // and looks here : at the same position, plus "direction"
								up                  // Head is up (set to 0,-1,0 to look upside-down)
						   );
	ViewMatrix=glm::translate(ViewMatrix, glm::vec3(0, stepOffset, 0));

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}