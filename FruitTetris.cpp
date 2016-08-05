/*
CMPT 361 Assignment 1 - FruitTetris implementation Sample Skeleton Code

- This is ONLY a skeleton code showing:
How to use multiple buffers to store different objects
An efficient scheme to represent the grids and blocks

- Compile and Run:
Type make in terminal, then type ./FruitTetris

This code is extracted from Connor MacLeod's (crmacleo@sfu.ca) assignment submission
by Rui Ma (ruim@sfu.ca) on 2014-03-04. 

Modified in Sep 2014 by Honghua Li (honghual@sfu.ca).
*/

#include "include/Angel.h"
#include <set>
#include <vector>
#include <iostream>
#include "roboarm.h"

using namespace std;

// misc constants



#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20

#define BOARD_POINTS 7200
#define GRID_POINTS 590
#define TILE_POINTS 144
#define CUBE_POINTS 36


 
//board[x][y] represents whether the cell (x,y) is occupied



//An array containing the colour of each of the 10*20*2*3 vertices that make up the board
//Initially, all will be set to black. As tiles are placed, sets of 6 vertices (2 triangles; 1 square)
//will be set to the appropriate colour in this array before updating the corresponding VBO


// xsize and ysize represent the window size - updated if window is reshaped to prevent stretching of the game
int xsize = 400; 
int ysize = 720;
int shapeToUse;
int orientation; //globally track which peice is being used, and which orientation it is in
int timerVal = 500;

mat4 M, V, P;

GLuint localMVP;


GLfloat  fovy = 80.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.1, zFar = 3.0;


// current tile
vec2 tile[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid
vec2 tilepos; // The position of the current tile using grid coordinates ((0,0) is the bottom left corner)


void setMVP(mat4 &mvp) {
	glUniformMatrix4fv(localMVP, 1, GL_TRUE, mvp);
}





vec2 allShapesAllRotations[6][4][4] =
	{	//Lshape
		{{vec2(-1,-1), vec2(-1, 0), vec2(0,0), vec2(1, 0)},
		{vec2(1, -1), vec2(0, -1), vec2(0,0), vec2(0,1)},     
		{vec2(1, 1), vec2(1,0), vec2(0, 0), vec2(-1,0)},  
		{vec2(-1,1), vec2(0, 1), vec2(0, 0), vec2(0,-1)}},
		//LshapeRightHanded
		{{vec2(1,-1), vec2(1, 0), vec2(0,0), vec2(-1, 0)},
		{vec2(-1, -1), vec2(0, -1), vec2(0,0), vec2(0,1)},     
		{vec2(-1, 1), vec2(-1,0), vec2(0, 0), vec2(1,0)},  
		{vec2(1,1), vec2(0, 1), vec2(0, 0), vec2(0,-1)}},
		//The S shapes make a slight difference to their rotation pattern, in that if an S shape, without moving down, 
		//is rotated twice, it will occupy the SAME space it did previously, instead of being translated up by one unit (and then back down in 2 more rotations)
		//Sshape
		{{vec2(-1, -1), vec2(0,-1), vec2(0, 0), vec2(1,0)},
		{vec2(1, -1), vec2(1, 0), vec2(0,0), vec2(0,1)},     
		{vec2(1,0), vec2(0,0), vec2(0,-1), vec2(-1, -1)},
		{vec2(0,1), vec2(0,0), vec2(1, 0), vec2(1, -1)}},
		//SshapeRightHanded
		{{vec2(1, -1), vec2(0,-1), vec2(0, 0), vec2(-1,0)},
		{vec2(-1, -1), vec2(-1, 0), vec2(0,0), vec2(0,1)},     
		{vec2(-1,0), vec2(0,0), vec2(0,-1), vec2(1, -1)},
		{vec2(0,1), vec2(0,0), vec2(-1, 0), vec2(-1, -1)}},
		//Ishape
		{{vec2(0, 1), vec2(0,0), vec2(0, -1), vec2(0,-2)},
		{vec2(-2, 0), vec2(-1, 0), vec2(0,0), vec2(1, 0)},     
		{vec2(0, -2), vec2(0,-1), vec2(0, 0), vec2(0, 1)},
		{vec2(1, 0), vec2(0, 0), vec2(-1,0), vec2(-2, 0)}},

		//allRotationsTshape
		{{vec2(-1, 0), vec2(0,0), vec2(1, 0), vec2(0,-1)},
		{vec2(0, -1), vec2(0, 0), vec2(0,1), vec2(1, 0)},     
		{vec2(1, 0), vec2(0,0), vec2(-1, 0), vec2(0,1)},
		{vec2(0, 1), vec2(0, 0), vec2(0,-1), vec2(-1, 0)}}};


vec4 allColors[5] = 
	{vec4(1.0, 0.5, 0.0, 1.0),
	vec4(1.0, 0.0, 0.0, 1.0),
	vec4(0.0, 1.0, 0.0, 1.0),
	vec4(0.73, 0.16, 0.96, 1.0),
	vec4(1.0, 1.0, 0.0, 1.0)};

vec4 orange = vec4(1.0, 0.5, 0.0, 1.0); 
vec4 red 	= vec4(1.0, 0.0, 0.0, 1.0);
vec4 green 	= vec4(0.0, 1.0, 0.0, 1.0);
vec4 purple	= vec4(0.73, 0.16, 0.96, 1.0); //values taken from https://www.opengl.org/discussion_boards/showthread.php/132502-Color-tables
vec4 yellow = vec4(1.0, 1.0, 0.0, 1.0);
vec4 white  = vec4(1.0, 1.0, 1.0, 0.5);
vec4 black  = vec4(0.0, 0.0, 0.0, 0.0); 


 
//board[x][y] represents whether the cell (x,y) is occupied
bool board[10][20]; 
vec4 boardcolours[BOARD_POINTS];
bool markedForDeletion[10][20];
vec4 original_colors[TILE_POINTS];
// When the current tile is moved or rotated (or created), update the VBO containing its vertex position data
// location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// locations of uniform variables in shader program
GLuint locxsize;
GLuint locysize;
GLuint locMVP;

// VAO and VBO
GLuint vaoIDs[3]; // One VAO for each object: the grid, the board, the current piece
GLuint vboIDs[6]; // Two Vertex Buffer Objects for each VAO (specifying vertex positions and colours, respectively)

void newtile();
void settile();
void updatetile();
void rotate();
bool detectCollision(GLfloat x, GLfloat y);


bool detectCollision(GLfloat x, GLfloat y){
	return (x < 0 || x >9 || y < 0 || y > 19 || board[int(x)][int(y)]);
}

// When the current tile is moved or rotated (or created), update the VBO containing its vertex position data
void updatetile()
{
	// Bind the VBO containing current tile vertex positions
	
	vec4 points[TILE_POINTS];
	vec4 greycube[CUBE_POINTS];

	vec4 colorcube[CUBE_POINTS];


	for (int i = 0; i< CUBE_POINTS; i++){
		greycube[i] = vec4 (0.5, 0.5, 0.5, 1.0);
	}
	// For each of the 4 'cells' of the tile,

	tilepos = roboarm::tip();
	for (int i = 0; i < 4; i++) 
	{
		for (int j = 0; j<CUBE_POINTS;  j++)
		{
			colorcube[j] = original_colors[i*CUBE_POINTS + j];
		}
		// Calculate the grid coordinates of the cell
		GLfloat x = tilepos.x + tile[i].x; 
		GLfloat y = tilepos.y + tile[i].y;

		// Create the 4 corners of the square - these vertices are using location in pixels
		// These vertices are later converted by the vertex shader


		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), 16.50, 1); // front left bottom
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), 16.50, 1); // front left top
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), 16.50, 1); // front right bottom
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), 16.50, 1); // front right top
		vec4 p5 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), -16.50, 1); // back left bottom
		vec4 p6 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), -16.50, 1); // back left top
		vec4 p7 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), -16.50, 1); // back right bottom
		vec4 p8 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), -16.50, 1); // back right top
/*
	  6---8		Front, Right, Back, Left
	 /|  /|		2---4		4---8		8---6		6---2
	2---4 |		| F |	,	| R |	,	| B |	,	| L |
	| 5-|-7 	1---3		3---7		7---5		5---1
	|/  |/ 
	1---3
				Top, Bottom
				6---8		1---3		
				| T |	,	| B |	
				2---3		5---7

				2 triangles for Front Face
				  
				2---4 		2
				 \	|		| \
				  \ |		|  \
				    3   	1---3
				Must specify in CCW order, so, 1,2,3 and 2,3,4 are the order for points 0-5 of the first block. 
		
*/			
			points[0] = p1;
			points[1] = p2; //front
			points[2] = p3;
			points[3] = p2;
			points[4] = p3;
			points[5] = p4;
			
			points[6] = p5; //right
			points[7] = p6;
			points[8] = p7;
			points[9] = p6;
			points[10] = p7;
			points[11] = p8;
			
			points[12] = p1; //back
			points[13] = p2;
			points[14] = p5;
			points[15] = p2;
			points[16] = p5;
			points[17] = p6;
			
			points[18] = p3; //left
			points[19] = p4;
			points[20] = p7;
			points[21] = p4;
			points[22] = p7;
			points[23] = p8;
			
			points[24] = p2; //top
			points[25] = p4;
			points[26] = p6;
			points[27] = p4;
			points[28] = p6;
			points[29] = p8;
			
			points[30] = p1; //bottom
			points[31] = p3;
			points[32] = p5;
			points[33] = p3;
			points[34] = p5;
			points[35] = p7;


			if (detectCollision(x, y)){
				cout<<"COLLISION DETECTED\n";
				glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
				glBufferSubData(GL_ARRAY_BUFFER, CUBE_POINTS*i*sizeof(vec4), CUBE_POINTS*sizeof(vec4), greycube); // Put the colour data in the VBO
			} else {
				glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
				glBufferSubData(GL_ARRAY_BUFFER, CUBE_POINTS*i*sizeof(vec4), CUBE_POINTS*sizeof(vec4), colorcube); // Put the colour data in the VBO
			}

			glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]); 
			glBufferSubData(GL_ARRAY_BUFFER, i*CUBE_POINTS*sizeof(vec4), CUBE_POINTS*sizeof(vec4), points);
		// Two points are used by two triangles each
		// Put new data in the VBO 
	}

	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------------------------
void snapdown() {

	bool set = false;
	bool canMove = true;
	tilepos = roboarm::tip();
	int lowpointOffset = 0;

	int xPosition;
	int yPosition;

	while (!set){
		//find low points
		for (int i = 0; i < 4; i++){
			xPosition = (int)tilepos.x + (int)tile[i].x;
			yPosition = (int)tilepos.y + (int)tile[i].y;
			if (board[xPosition][yPosition-1]) {//True if the space is occupied
				cout<< "The space below me is occupied!"<<endl;
				canMove=false;
			}
		}
		if (canMove){ //nothing underneath
			tilepos.y--; //see if we have hit the bottom
			cout<< "The space below me is NOT occupied!"<<endl;
			for (int i = 0; i<4; i++){
				lowpointOffset = min((int)tile[i].y, lowpointOffset); 
			}
			if (tilepos.y <= abs(lowpointOffset)) {
				set = true;
			}
		} else {
			set = true;
		}
		if (set) {
			
			settile();
			newtile();
		}
	}
	glutPostRedisplay();
}
// Called at the start of play and every time a tile is placed
void newtile() {
	// Update the geometry VBO of current tile
	shapeToUse = rand()%6; //random number between 0 and 5 to pick a shape
	orientation = rand()%4; //random number ... to pick the orientation
	int position =  rand()%8 + 1; //random number ... to pick the horizontal spawn position -- pivotpoint shouln't be at x=0

	//check if it is in the bounds
	//check if it is colliding with anything
	vec2 tilepos = roboarm::tip();
	// Update the geometry VBO of current tile
	for (int i = 0; i < 4; i++)
		tile[i] = allShapesAllRotations[shapeToUse][orientation][i];
	updatetile(); 


	
	// Update the color VBO of current tile
	vec4 newcolours[TILE_POINTS];
	int color = 1;//rand()%5;
	for (int i = 0; i < TILE_POINTS; i++) {
		if (i % CUBE_POINTS == 0){
			color = rand()%5;
		}
		newcolours[i] = allColors[color];
		original_colors[i] = allColors[color];
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
	glBufferSubData(GL_ARRAY_BUFFER, 0, TILE_POINTS*sizeof(vec4), newcolours); // Put the colour data in the VBO
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------

void initGrid() {
	// 462 = 21*11*2 is depth lines
	vec4 gridpoints[64*2 + 462];
	vec4 gridcolours[64*2 + 462];
	// Vertical lines 
	for (int i = 0; i < 11; i++){
		gridpoints[2*i]      = vec4((33.0 + (33.0 * i)), 33.0, 16.50, 1);
		gridpoints[2*i + 1]  = vec4((33.0 + (33.0 * i)), 693.0, 16.50, 1);
		gridpoints[2*i + 64] = vec4((33.0 + (33.0 * i)), 33.0, -16.50, 1);
		gridpoints[2*i + 65] = vec4((33.0 + (33.0 * i)), 693.0, -16.50, 1);
	}
	// Horizontal lines
	for (int i = 0; i < 21; i++){
		gridpoints[22 + 2*i] 		= vec4(33.0, (33.0 + (33.0 * i)), 16.50, 1);
		gridpoints[22 + 2*i + 1] 	= vec4(363.0, (33.0 + (33.0 * i)), 16.50, 1);
		gridpoints[22 + 2*i + 64]	= vec4(33.0, (33.0 + (33.0 * i)), -16.50, 1);
		gridpoints[22 + 2*i + 65] 	= vec4(363.0, (33.0 + (33.0 * i)), -16.50, 1);
	}
	// Depth lines
	for (int i = 0; i < BOARD_HEIGHT + 1; i++){
		for (int j = 0; j < BOARD_WIDTH + 1; j++) {
			gridpoints[128 + 22*i + 2*j] 		= vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), 16.50, 1); // front left bottom
			gridpoints[128 + 22*i + 2*j + 1] 	= vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), -16.50, 1); // back left bottom
		}
	}
	// Make all grid lines coloured
	for (int i = 0; i < 64*2 + 462; i++)
		gridcolours[i] = white;


	// *** set up buffer objects
	// Set up first VAO (representing grid lines)
	glBindVertexArray(vaoIDs[0]); // Bind the first VAO
	glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours)

	// Grid vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]); // Bind the first grid VBO (vertex positions)
	glBufferData(GL_ARRAY_BUFFER, GRID_POINTS*sizeof(vec4), gridpoints, GL_STATIC_DRAW); // Put the grid points in the VBO
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(vPosition); // Enable the attribute
	
	// Grid vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]); // Bind the second grid VBO (vertex colours)
	glBufferData(GL_ARRAY_BUFFER, GRID_POINTS*sizeof(vec4), gridcolours, GL_STATIC_DRAW); // Put the grid colours in the VBO
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor); // Enable the attribute
}

void initBoard() {
	vec4 boardpoints[BOARD_POINTS];
	int offset;
	for (int i = 0; i < BOARD_POINTS; i++){
		boardcolours[i] = black;
	} // Let the empty cells on the board be black
	// Each cell is a square (2 triangles with 6 vertices)
	for (int y = 0; y < 20; y++){
		for (int x = 0; x < 10; x++)
		{		
			offset = 0;
			vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), 16.5, 1); 
			vec4 p2 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), 16.5, 1);
			vec4 p3 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), 16.5, 1);
			vec4 p4 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), 16.5, 1);
			vec4 p5 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), -16.5, 1); 
			vec4 p6 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), -16.5, 1);
			vec4 p7 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), -16.5, 1);
			vec4 p8 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), -16.5, 1);

/*
	  8---7		Front, Right, Back, Left
	 /|  /|		4---3		3---7		7---8		8---4
	4---3 |		| F |	,	| R |	,	| B |	,	| L |
	| 5-|-6 	1---2		2---6		6---5		5---1
	|/  |/ 
	1---2
				Top, Bottom
				8---7		1---2		
				| T |	,	| B |	
				4---3		5---6

				2 triangles for Front Face
				  
				    3 		4---3
				  /	|		|  /
				 /  |		| /
				1---2   	1 
				Must specify in CCW order, so, 1,2,3 and 3,4,1 are the order for points 0-5 of the first block. 

*/
			offset = CUBE_POINTS*(10*y + x);

			boardpoints[offset + 0] = p1;
			boardpoints[offset + 1] = p2; //front
			boardpoints[offset + 2] = p3;
			boardpoints[offset + 3] = p3;
			boardpoints[offset + 4] = p4;
			boardpoints[offset + 5] = p1;
			
			boardpoints[offset + 6] = p2; //right
			boardpoints[offset + 7] = p6;
			boardpoints[offset + 8] = p7;
			boardpoints[offset + 9] = p7;
			boardpoints[offset + 10] = p3;
			boardpoints[offset + 11] = p2;
			
			boardpoints[offset + 12] = p6; //back
			boardpoints[offset + 13] = p5;
			boardpoints[offset + 14] = p8;
			boardpoints[offset + 15] = p8;
			boardpoints[offset + 16] = p7;
			boardpoints[offset + 17] = p6;
			
			boardpoints[offset + 18] = p5; //left
			boardpoints[offset + 19] = p1;
			boardpoints[offset + 20] = p4;
			boardpoints[offset + 21] = p4;
			boardpoints[offset + 22] = p8;
			boardpoints[offset + 23] = p5;
			
			boardpoints[offset + 24] = p4; //top
			boardpoints[offset + 25] = p3;
			boardpoints[offset + 26] = p7;
			boardpoints[offset + 27] = p7;
			boardpoints[offset + 28] = p8;
			boardpoints[offset + 29] = p4;
			
			boardpoints[offset + 30] = p5; //bottom
			boardpoints[offset + 31] = p6;
			boardpoints[offset + 32] = p2;
			boardpoints[offset + 33] = p2;
			boardpoints[offset + 34] = p1;
			boardpoints[offset + 35] = p5;
			
		}
	}

	// Initially no cell is occupied
	for (int i = 0; i < 10; i++){
		for (int j = 0; j < 20; j++){
			board[i][j] = false; 
		}
	}


	// *** set up buffer objects
	glBindVertexArray(vaoIDs[1]);
	glGenBuffers(2, &vboIDs[2]);

	// Grid cell vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, BOARD_POINTS*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, BOARD_POINTS*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

// No geometry for current tile initially
void initCurrentTile() 
{
	glBindVertexArray(vaoIDs[2]);
	glGenBuffers(2, &vboIDs[4]);

	// Current tile vertex positions

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
	glBufferData(GL_ARRAY_BUFFER, TILE_POINTS*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Current tile vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferData(GL_ARRAY_BUFFER, TILE_POINTS*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void init() {
	// Load shaders and use the shader program
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// Get the location of the attributes (for glVertexAttribPointer() calls)
	vPosition = glGetAttribLocation(program, "vPosition");
	vColor = glGetAttribLocation(program, "vColor");

	// Create 3 Vertex Array Objects, each representing one 'object'. Store the names in array vaoIDs
	glGenVertexArrays(3, &vaoIDs[0]);

	// Initialize the grid, the board, and the current tile
	initGrid();
	initBoard();
	initCurrentTile();
	roboarm::init();
	roboarm::Theta[roboarm::LowerArm] = 5;
	roboarm::Theta[roboarm::UpperArm] = -85;

	// The location of the uniform variables in the shader program
	locMVP = glGetUniformLocation(program, "MVP");

	// Board is now in unit lengths
	vec3 eye = vec3(0, 30, 50);
	vec3 at = vec3(0, 10, 0);
	vec3 up = vec3(0, 1, 0);
	V = LookAt(eye, at, up);
	
	newtile(); // create new next tile

	// Blend
   	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0, 0, 0, 1);
	// Depth
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0);
	// Antialiasing
	glEnable(GL_MULTISAMPLE);
	glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
}


//-------------------------------------------------------------------------------------------------------------------

// Given (x,y), tries to move the tile x squares to the right and y squares down
// Returns true if the tile was successfully moved, or false if there was some issue
bool movetile(vec2 direction)
{
	//concept: Check boundaries, then write changes, then update tile
	//check boundaries by finding the NEXT step most minimum/maximum x/y values
	int left=10, right=-1; //boundary trackers
	int xPosition, yPosition;
	bool overlap = false;
	for (int i = 0; i < 4; i++){
		//deterime if there is overlap
		xPosition = (int)tilepos.x + (int)tile[i].x + (int)direction.x;
		yPosition = (int)tilepos.y + (int)tile[i].y + (int)direction.y;

		if(board[xPosition][yPosition]) //if ANY of the pieces overlap, then overlap is true and we will NOT MOVE
			overlap = true;

		left = min(xPosition, left);
		right = max(xPosition, right);

	}
	if (left < 0){ //the direction was left, and it would take us out of the grid
		return false;
	}
	if (right > 9){ //the direction was right and it would take us out of the grid
		return false;
	}
	if (!overlap){ //if there is NO overlap
		tilepos.x += direction.x;
		updatetile();
		return true;
	}
	//at this point, we are within the bounds of the gridbox. Check if we overlap with any existing tiles!

	return false;
}
//-------------------------------------------------------------------------------------------------------------------


// Starts the game over - empties the board, creates new tiles, resets line counters
void restart()
{
	init();
}
//-------------------------------------------------------------------------------------------------------------------
void rotate() //global variables shapetouse and orientation indicate which shape is being used, and which orientation it is in
{     
	int originalOrientation = orientation;

	if (++orientation > 3) //increment the orientation. If it is larger than 3, then we go back to 0.
		orientation=0;

	int xPosition, yPosition;
	int left = 10;
	int right= -1;
	bool overlap = false;

	for (int i = 0; i < 4; i++){ //check the bounds on the potential NEW orientation
		xPosition = (int)tilepos.x + (int)allShapesAllRotations[shapeToUse][orientation][i].x;
		yPosition = (int)tilepos.y + (int)allShapesAllRotations[shapeToUse][orientation][i].y;

		if(board[xPosition][yPosition]) //if ANY of the pieces overlap, then overlap is true and we will NOT MOVE
			overlap = true;

		left = min(xPosition, left);
		right = max(xPosition, right);

	}
	if (left < 0){ //the direction was left, and it would take us out of the grid
		orientation = originalOrientation;
		return;
	}
	if (right > 9){ //the direction was right and it would take us out of the grid
		orientation = originalOrientation;
		return;
	}

	if (!overlap){ //if there is NO overlap
		for (int i = 0; i < 4; i++){
			tile[i] = allShapesAllRotations[shapeToUse][orientation][i];
		}
		updatetile();
		return;
	}
	orientation = originalOrientation;
	return;
}

// Draws the game
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	P = Perspective(45, 1.0*xsize/ysize, 10, 200);

	// Draw the robot
    glBindVertexArray(roboarm::vao);
	mat4 f = P * V * Translate(roboarm::pos);
	roboarm::robotMVP = RotateY(roboarm::Theta[roboarm::Base] );
	roboarm::base(f);

	roboarm::robotMVP *= Translate(0.0, roboarm::BASE_HEIGHT, 0.0);
	roboarm::robotMVP *= RotateZ(roboarm::Theta[roboarm::LowerArm]);
	roboarm::lower_arm(f);

	roboarm::robotMVP *= Translate(0.0, roboarm::LOWER_ARM_HEIGHT, 0.0);
	roboarm::robotMVP *= RotateZ(roboarm::Theta[roboarm::UpperArm]);
	roboarm::upper_arm(f);

	roboarm::robotMVP *= Translate(0.0, roboarm::UPPER_ARM_HEIGHT, 0.0);

	// Scale everything to unit length
	mat4 M = mat4();
	M *= Translate(0, BOARD_HEIGHT/2.0, 0);
	M *= Scale(1.0/33, 1.0/33, 1.0/33);  // scale to unit length
	M *= Translate(-33*BOARD_WIDTH/2.0 - 33, -33*BOARD_HEIGHT/2.0 - 33, 0); // move to origin

	mat4 MVP = P * V * M;
	setMVP(MVP);

	glBindVertexArray(vaoIDs[1]); // Bind the VAO representing the grid cells (to be drawn first)
	glDrawArrays(GL_TRIANGLES, 0, BOARD_POINTS); // Draw the board (10*20*2 = 400 triangles)

	glBindVertexArray(vaoIDs[2]); // Bind the VAO representing the current tile (to be drawn on top of the board)
	glDrawArrays(GL_TRIANGLES, 0, 24*6); // Draw the current tile (8 triangles)

	glBindVertexArray(vaoIDs[0]); // Bind the VAO representing the grid lines (to be drawn on top of everything else)
	glDrawArrays(GL_LINES, 0, 128 + 462);

	
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, BOARD_POINTS*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	
	glutSwapBuffers();
}

// Reshape callback will simply change xsize and ysize variables, which are passed to the vertex shader
// to keep the game the same from stretching if the window is stretched
void reshape(GLsizei w, GLsizei h) {
	xsize = w;
	ysize = h;
	glViewport(0, 0, w, h);
}

// Handle arrow key keypresses
void special(int key, int x, int y) {
	switch(key) {
		case GLUT_KEY_UP:
			if(glutGetModifiers() == GLUT_ACTIVE_CTRL)
				V *= RotateZ(10);
			else {
				rotate();
			}
			break;
		case GLUT_KEY_RIGHT:
			if(glutGetModifiers() == GLUT_ACTIVE_CTRL)
				V *= RotateY(10);
			break;
		case GLUT_KEY_LEFT:
			if(glutGetModifiers() == GLUT_ACTIVE_CTRL)
				V *= RotateY(-10);
			break;
		default:
			break;
	}
}

// Handles standard keypresses
void keyboard(unsigned char key, int x, int y) {
	// various test cases. press t or z to find out!
	bool collision = false;
	switch(key) 
	{
		case 033: // Both escape key and 'q' cause the game to exit
		    exit(EXIT_SUCCESS);
		    break;
		case 'q':
			exit (EXIT_SUCCESS);
			break;
		case 'r': // 'r' key restarts the game
			restart();
			break;
		case 'a':
			roboarm::Theta[roboarm::LowerArm] += 5;
			updatetile();
			break;
		case 'd':
			roboarm::Theta[roboarm::LowerArm] -= 5;
			updatetile();
			break;
		case 'w':
			roboarm::Theta[roboarm::UpperArm] += 5;
			updatetile();
			break;
		case 's':
			roboarm::Theta[roboarm::UpperArm] -= 5;
			updatetile();
			break;
		case ' ':
			for (int i = 0; i<4; i++){
				if (collision || detectCollision(tilepos.x + tile[i].x, tilepos.y + tile[i].y)){ //collision is already foun or foun now
					cout<<"RUDE. You can't play when it's grey!\n";
					collision = true;
				}
			} if (collision){
				
			} else {
				snapdown();
			}
			break;
	}
	glutPostRedisplay();
}

void idle(void) {
	glutPostRedisplay();
}


// Checks if the specified row (0 is the bottom 19 the top) is full
// If every cell in the row is occupied, it will clear that cell and everything above it will shift down one row
void checkfullrow(int row)
{
	bool rowFull=true;
	for (int i = 0; i < 10; i++){
		if (!board[i][row]){
			rowFull=false;
			i=10;
		}
	}
	if (rowFull){
		for(int i=row; i<19; i++){
			for(int j=0; j<10; j++){
				board[j][i]=board[j][i+1];
				for (int k = 0; k < 6; k++){
					boardcolours[60*i + 6*j + k]=boardcolours[60*i + 60 + 6*j + k];
				}
			}
		}
		for(int j = 0; j< 10; j++){
			board[j][19]=false;
			for (int k = 0; k < 6; k++){
					boardcolours[60*19 + 6*j + k] = black; 
				}
		}
		checkfullrow(row); //in the event that two consecutive rows are formed, the previous mechanism for calling checkfullrow will miss this row
	}
}

//-------------------------------------------------------------------------------------------------------------------

// main loop that handles the moving down of the tile and other game logic

bool compareCol(vec4 col1, vec4 col2)
{
	return ((col1.x == col2.x ) && (col1.y == col2.y) && (col1.z == col2.z));
}
void removeMarked() 
{
	//start from the TOP, delete a block at a time, move those above it down, keep going.
	for (int row = 19; row > -1; row--){
		for (int col = 0; col < 10; col++){
			if (markedForDeletion[col][row]){
				for(int i=row; i<19; i++){
					board[col][i]=board[col][i+1];
					for (int k = 0; k < 6; k++){
						boardcolours[60*i + 6*col + k]=boardcolours[60*i + 60 + 6*col + k];	
					}
				}
				markedForDeletion[col][row] = false;
			}
		}
	}
}

void checkforthree()
{
	vec4 currentColor;
	int length =0;
	bool anythingFound = false;
	for (int color = 0; color < 5; color++){ //check the board entirely for each color
		currentColor = allColors[color];	
		for (int row = 0; row < 20; row++){
			for (int col = 0; col < 10; col++){
				if (compareCol(boardcolours[row*60 + col*6],currentColor)){

					/*-----Look to the Right-----*/
					length=0;					
					while (compareCol(boardcolours[row*60 + (col + length)*6],currentColor)) {			
						length++;
					}

					if (length >= 3) { //this could probably be done better, but I couldn't figure something out
						for (int i = 0; i < length; i++){
							markedForDeletion[col + i][row] = true;
						}
						anythingFound = true;						
					} 
					

					/*-----Look Above-----*/
					length=0;					
					while (compareCol(boardcolours[(row + length)*60 + col*6],currentColor)) {			
						length++;
					}

					if (length >= 3) { //this could probably be done better, but I couldn't figure something out
						for (int i = 0; i < length; i++){
							markedForDeletion[col][row + i] = true;
						}
						anythingFound = true;						
					}

					/*-----Look Diagonally (up and right)----*/
					length=0;					
					while (compareCol(boardcolours[(row + length)*60 + (col + length)*6],currentColor)) {			
						length++;
					}

					if (length >= 3) { //this could probably be done better, but I couldn't figure something out
						for (int i = 0; i < length; i++){
							markedForDeletion[col + i][row + i] = true;
						}
						anythingFound = true;						
					}


					/*-----Look Diagonally (up and left)----*/
					length=0;					
					while (compareCol(boardcolours[(row + length)*60 + (col - length)*6],currentColor)) {			
						length++;
					}

					if (length >= 3) { //this could probably be done better, but I couldn't figure something out
						for (int i = 0; i < length; i++){
							markedForDeletion[col - i][row + i] = true;
						}
						anythingFound = true;						
					}
				}
			}
		}
	}
	if(anythingFound){
		removeMarked();
		checkforthree();
	}
}

//-------------------------------------------------------------------------------------------------------------------
// Places the current tile - update the board vertex colour VBO and the array maintaining occupied cells
void settile()
{
		int xPosition, yPosition, start, tracker;
		vec4 tilecolours[TILE_POINTS];
		timerVal = 500;
		glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
		glGetBufferSubData(GL_ARRAY_BUFFER,0,TILE_POINTS*sizeof(vec4),tilecolours);
		glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
		for (int i = 0; i < 4; i++){
			xPosition = (int)tilepos.x + (int)tile[i].x;
			yPosition = (int)tilepos.y + (int)tile[i].y;
			board[xPosition][yPosition] = true;
			start = CUBE_POINTS*(yPosition*10 + xPosition);
			tracker = 0;
			while (tracker < CUBE_POINTS){
				boardcolours[start + tracker] = tilecolours[CUBE_POINTS*i + tracker];
				tracker++;
			}
			glBufferSubData(GL_ARRAY_BUFFER, 0, BOARD_POINTS*sizeof(vec4), boardcolours);
		}
		//start = min(19, (int)tilepos.y + 2); 	//Start at the highest potentially affected row (give or take), would likely work as +1
		//end = max((int)tilepos.y - 3, -1);	
		
		//for (int i = start; i > end; i--){
			//checkfullrow(i);
		//}
		//checkforthree();
	
}


int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_MULTISAMPLE | GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(xsize, ysize);
	glutInitWindowPosition(680, 178); // Center the game window (well, on a 1920x1080 display)
	glutCreateWindow("Fruit Tetris");
	glewInit();
	init();

	// Callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);

	glutMainLoop(); // Start main loop
	return 0;
}
