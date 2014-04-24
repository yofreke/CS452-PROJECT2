CS452-PROJECT2
==============
Due Friday April 18


This project was built ontop of a tutorial series offered online at http://www.opengl-tutorial.org/.

We used the following premade classes from the tutorial series:
- shader.cpp
- texture.cpp
- controls.cpp
- objloader.cpp

Edit summary: controls.cpp
	This class offered nothing in the way of gravity to begin with, and offered no form of collision detection.  We added this functionality by adding the functions:
	
	- int mapCoord(int mapSize, glm::vec3 pos)
	- int getMapIdAt(int* map, int mapSize, glm::vec3 pos)
	- bool isValid(int* map, int mapSize, glm::vec3 pos)
	
	This allowed us to check if the position moved to in a frame was valid.  This is one of the most simple types of collision detections (valid position -> temporary position -> if temporary is valid set temporary position as new valid position else do not update the current valid position)

	Gravity is simple and assumes that the ground is a flat plane at y = 0.

	We also had to edit the default controls to not lock the mouse cursor (first person control), but instead adapt a classic dungeon crawler feel (arrow keys to rotate).

Edit summary: objloader.cpp
	While none of the edits made to this class made it to the final version, I will include a brief summary of the functionality because edits were made to this class in the development process.

	An obj file consists of four sections:
	- vertex locations
	- vertex texture locations
	- vertex normals
	- faces

	The objloader takes a file, and parses the first three sections into vectors of types vec3, vec2, and vec3.  Then it parses the faces, and creates the final indice list, uv list, and normal list.

We also used a maze generator which can be found at http://www.roguebasin.com/index.php?title=Simple_maze
	Edits had to be made to this class, but they were minimal and hacky.  Essentially we just edit the generator to spit out an array of integers where 0 is not a wall and 1 is a wall.

The main class functions as follows:
	- Setup opengl and glfw
		Make context, enable things, set input modes, etc.
	- Load and setup shaders
	- Initilize the basic cube obj
	- Reset the map
		- Generate a new integer array using the maze generator
		- Clear verticies, uvs, normals, and list of spawned chests
		- Iterate through each of the map tiles and add the non adjacent sides to the map render lists.  This is done by testing if a tile has a North neighbor, an East neighbor, etc.  If there is no neighbor, the 6 verticies, uvs, and normals are added to the map lists.  The verticies are offset based on their tile location.
			- There is a chance to set a chest at each tile.  A new chest struct is created (position and rotation), and then placed at the specified tiles world position, given a randomized rotation, and added to the spawned chests list.
		- Lastly the buffers are passed to the graphics card using opengl
	- Load the generic chest obj
	- Enter the main game loop
		- Test for chests within a given distance from the current camera position.  If there is one remove the chest from the map.  If the removed chest is the last, add to the map size and reset the map.
		- Adjust the world and view matricies based on input (and check for collisions).
		- Draw the world
		- Iterate through all of the chests in the map, get a model matrix for each chest based on the positions and rotations kept in the vector of spawnedChests.
		- Render current chest

NOTE: The lose condition of this game is when you run out of time.  In real life.  When you die as a human being.  The game goes on forever.