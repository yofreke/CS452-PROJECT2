#ifndef CONTROLS_HPP
#define CONTROLS_HPP

glm::vec3 getCamPos();
void setCamPos(glm::vec3);

void computeMatricesFromInputs(int* map, int mapSize);
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();

#endif