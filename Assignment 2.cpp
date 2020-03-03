//Vytenis Sakalinskas

#include <math.h> //I use this for sqrt();
#include <sstream> //Using this for the Draw() functionality
#include <string> //I use this for to_string
#include <iostream> //Using this for the draw() outputs
#include <TL-Engine.h>	// TL-Engine include file and namespace
using namespace tle;
I3DEngine* myEngine;

//Classes have to be put on top because they are blueprints and will not be usable if they are defined in main
//Which I did to begin with which prevented me from using the class data type as a type in my functions
//Originally I was using classes but then I ended up using structs, I was making a class and making it public and there's no reason to waste lines when I could just use a struct instead
//I ended up using structs since they could hold the data relevant to the game object and I wouldn't need to make tons of new variables that could easily get lost if they weren't attached to an object
struct Rover
{
	IModel* rover = 0;
	float timer = 2;
	bool timerActive = false;
};
Rover rover1;
Rover rover2;
Rover rover3;
Rover mixedRover1;

//I should have used a frog object as well, the frog was the first part of the game that I made and by the time I got comfortable with structs it was too late to go back without changing
//extreme amounts of code
//The structs here are used to store the model associated with the object, a timer that counts down when the cars leave the screen and a timer to indicate if the timer should be counting down
//this is used for both rovers and transits
struct Transit
{
	IModel* transit = 0;
	float timer = 2;
	bool timerActive = false;
};
Transit transit1;
Transit transit2;
Transit transit3;
Transit mixedTransit1;

//Tyres don't need a timer like the cars do, all they need is the model and a single boolean, I use this boolean to change the directions of the tyres after they hit a bound
struct Tyre
{
	IModel* tyre = 0;
	bool MovingRight = false;
};
Tyre tyre1;
Tyre tyre2;
Tyre tyre3;
Tyre tyre4;

const float kGameSpeed = 10.0f; //The classic game speed, need that to have a time at which the game moves at, across the board
const float kFrogRadius = 2.5f; //The radius that is used to indicate how much additional size the cars should have for the point to square collision on cars
const float kResetZone = 50.0f; //The reset zone is used to as a bounding limit to the cars movement, if they move beyond the zone or the - version of the zone the cars go into limbo
const float kCameraPositiveCap = 60.0f; //The bound for the camera moving up, it's used to prevent the camera from going indefinitely
const float kCameraNegativeCap = -10.0f; //same thing as the one above but negative
const float kTyreRadius = 5.0f; //The radius of the tyres used in the sphere to sphere collision for the box

enum frogDirections { Forward, Backwards, Left, Right }; //The directions of the frog that I use to turn the frogs

//The default states of the frogs directions on game start
frogDirections frog1Direction = Forward;
frogDirections frog2Direction = Forward;
frogDirections frog3Direction = Forward;

enum frogStates { Waiting, Crossing, Safe, Dead }; //The states that a frog can be through the span of the game

//The default states of the frogs at the start of the game
frogStates frog1State = Crossing;
frogStates frog2State = Waiting;
frogStates frog3State = Waiting;

enum gameStates { Playing, Paused, Over }; //The states that the game can be in
gameStates currentGameState = Playing; //Starting game state

const string kDeadImageName = "frog_red.jpg"; //The string I use to change the skin for the frogs when they dies
const string kSafeImageName = "frog_blue.jpg"; //The string I use to change the skin for the frogs when they are deemed safe

const int kScale = 10; //The scale int is used for increasing the size of the tyres
const float deadFrogScale = 0.4f; //The int that I use to scale down the frogs when they die

//The & is used because I am passing by reference and not copy since I am storing all the data inside the model variable, without using & it would create a copy of the IModel
//And store the data inside of the copy instead of the actual model so when I attempt to access the variables inside of it, the program would cause an access violation because
//The original variable model didn't get any data
//Default parameters, set position to always be 0,0,0 unless stated otherwise in main function 

//I used this function to speed up the leading of models, instead of having to write a bunch of code for each one I could just throw this into a loop with 10x less space used
void LoadInModel(IMesh* meshName, IModel*& modelName, string loadMeshName, float modelPosX = 0, float modelPosY = 0, float modelPosZ = 0);

//The function used to move cars, it checks if they're in limbo and moves them
void MoveCarModels(IModel*& modelName, float speedMultiplier, float frametime);

//One of the most important functions, it checks if the frog has collided and what to do if it did, I created a function inside of a function and I quickly learned how much of a mistake that was
//Sadly it was too late to go back on it since so much depended on it
void CheckCollision(int& currentFrogNumber, IModel*& currentFrog, IModel*& nextFrog, float& carMinX, float& carMaxX, float& carMinZ, float& carMaxZ, IModel*& dummy, float& distanceMoved, bool& frogIsMoving, bool& frogIsMovingSideways, bool& movingTheFrog, int& countdowntimer, bool& frogIsSafe);

//The function I used to send transits into limbo and from limbo, to count down the timer and to change the boolean that enables the timer
void TransitLimbo(Transit& transit, float frametime, float kResetZone, float limboPosition);

//Same as transit limbo, put them into limbo, count down timer, enable and disable boolean
void RoverLimbo(Rover& rover, float frametime, float kResetZone, float limboPosition);

//This one I used to change the boolean and count down the timer for mixed cars only, since they're different data types of cars, one Rover and one Transit type they needed to be in a different check
void MixedRoverLimbo(Rover& rover, float frametime, float kResetZone, float limboPosition);

//Checking the collision with the tyre and the frog using sphere to sphere collision, attaches to the tyre if at the end of the movement they both are in collision
void CheckTyreCollision(IModel* frogModel, IModel* tyreModel, bool& collisionWithTyre, IModel*& dummy, float kFrogStartingPositionsX, float kFrogStartingPositionsZ, float  kTyrePosY, frogDirections frogDirection, int& currentTyreCounter);

//What I used to rotate and move my frog, or dummy in this case since I attach the frog to the dummy and move that
void FrogMovement(IModel*& dummy, float& frametime, bool& frogIsMovingSideways, float& distanceMoved, bool& frogIsMoving, float& distanceTraveled, float kGameSpeed, bool& movingTheFrog, float kFrogSpeedMultiplier);

//Frog ripped means the frog died, I could change it but ripped sounds better, this does all the resetting and moving onto the next frog that has to happen after the frogs die
void FrogRipped(int& currentFrogNumber, IModel*& currentFrog, IModel*& nextFrog, IModel*& dummy, float& distanceMoved, bool& frogIsMoving, bool& frogIsMovingSideways, bool& movingTheFrog, bool& frogIsSafe);

//This is what I use to allow the frohs to move away from the tyre parents they are attached to, this resets their scale and positions and counters to prevent the frogs being in the wrong positions and bieng the wrong sizes
void DetachFromTyre(IModel*& dummy, IModel*& tyreModel, float kFrogStartingPositionsX, float kTyrePosY, float kFrogStartingPositionsZ, bool& collisionWithTyre, int& currentTyreCounter, frogDirections frogDirection);

//Instead of having to use 4 lines of code for every single objects collision box I ended up using a function to find out the collision box easier and faster, saved about a 100 lines of code if not more, about 12 per car
void GetCoordinates(IModel*& car, float carX, float carZ, float& carMinX, float& carMaxX, float& carMinZ, float& carMaxZ);

const float kRightSideRotation = -90.0f; //The rotation that is needed for the transits to be on the lane correctly
const float kLeftSideRotation = 90.0f; //The negative rotation that is needed for the rovers to be in the lane correctly

const string kSkyboxName = "skybox.x"; //The string that is used to store the name of the file that I store into the mesh
const float kSkyboxPosY = -1000.0f; //The position for the skybox

const string kSurfaceName = "surface.x"; //String used to store the name of the file I'll store into the mesh
const float kSurfacePosY = -5.0f; //Position of the water

const string kIsland1Name = "island1.x"; //The string used to store the name of the file I'll store into the mesh
const float kIsland1PosY = -5.0f; //The X orientation for the island
const float kIsland1PosZ = 40.0f; //the y orientation for the island

const float kCameraPositionY = 40.0f; //The starting Y position used when creating the camera
const float kCameraPositionZ = -60.0f; //The starting Z position used when creating the camera

const string kFrogName = "frog.x"; //String used to store the name of the file I'll store into the mesh

const string kTransitName = "transit.x"; //String used to store the name of the file I'll store into the mesh
const float kTransitPosZ = 35.0f; //Starting Z position of the transit

const string kRoverName = "rover.x"; //String used to store the name of the file I'll store into the mesh
const float kRoverPosZ = 45.0f; //starting Z position of the rover

const string kDummyName = "dummy.x"; //String used to store the name of the file I'll store into the mesh

const string kIsland2Name = "island2.x"; //String used to store the name of the file I'll store into the mesh
const float kIsland2PosY = -5.0f; //starting Y position of the second island
const float kIsland2PosZ = 115.0f; //starting Z position of the second island

const float kTextPosition1 = 50.0f; //Position used for the text coordinates, this gets used for both x and y coordinates
const float kTextPosition2 = 1150.0f; //Position used for the X text coordinate
const float kTextOverX = 600.0f; //the x position for the game over text
const float kTextOverY = 200.0f; //the y position for the game over text

const float kMinWater = 70.0f; //The bounding minimum limit for the water to become a kill plane 
const float kMaxWater = 110.0f; //the bounding maximum limit after which anything after it does not work as a kill plane and turns the frog into a safe one

const string kFunnyHaha = "h.jpg"; //Hehe

const string kTyreName = "tyre.x"; //String used to store the name of the file I'll store into the mesh

//An array of the starting X positions of all the tyres
const float kTyrePosX[4]
{
	5.0f,
	10.0f,
	15.0f,
	20.0f
};
const float kTyrePosY = -2.5f; //All the tyres share the same Y position so an array is not needed

//The z positions for the tyres, since they are all at a difference of 10 I can use a single variable next time and incriment it by 10 for each stage of a loop instead
const float kTyrePosZ[4]
{
	75.0f,
	85.0f,
	95.0f,
	105.0f,
};

//The starting positions for the cars, the positive versions are used for the transits while the negative ones are used for rovers, the positive ones are used for the mixed cars
//since the cars start at the same position I only need 3 variables, I could even use a single one next time and incriment it by 30 for each stage of the loop
const float kCarStartingPositionX[3] =
{
	{10.0f},
	{40.0f},
	{70.0f}
};

const float kTyreSpeed = 1.0f; //The adjustable speed for the tyres

const float kMixedPosZ = 55.0f; //The Z positions for the cars in the mixed list

const EKeyCode kExitKey = Key_Escape; //Key used to close out the game
const EKeyCode kPauseKey = Key_P; //Key used to pause the game
const EKeyCode kMoveLeft = Key_Z; //Key used to move the frog left
const EKeyCode kMoveRight = Key_X; //Key used to move the frog right

//the ' key used to move the frog forward, some poeple had the issue with this not once in a while
//This would occur because pressing shift and alt would change the language on visual studio
//even if your pc doesn't have different keyboards visual studio does and the key ' on the other type of keyboard doesn't match up with this keycode
const EKeyCode kMoveForward = EKeyCode(192);

const EKeyCode kMoveBackwards = EKeyCode(191); //Key used to move the frog back
const EKeyCode kRotateUp = EKeyCode(Key_Up); //Key used to move the camera up and rotate it
const EKeyCode kRotateDown = EKeyCode(Key_Down); //Key used to move the camera down and rotate it
const EKeyCode kResetCamera = EKeyCode(Key_C); //Key used to reset the camera

const float kTransitSpeedMultiplier = 1.0f; //The adjustable speed multiplier for the transits
const float kRoverSpeedMultiplier = 1.5f; //the adjustabale speed multiplier for the rovers
const float kMixedCarSpeedMultiplier = 1.5f; //the adjustable speed multiplier for mixed cars, while the values of rover and mixed are the same it might change later so I needed two variables
const float kFrogSpeedMultiplier = 2.5f; //the adjustable speed multiplier for the frog, I made it faster than the original one since it's more enjoyable

const int kBaseCountdown = 20; //The value used to reset the countdown back to it's original value
const int kCurrentFrogLimit = 2; //the value used to indicate at which point the game should go into the game over state

//a multidimensional arrahy of the starting frog coordinates, the ones in [x][0] are x coordinates and those in [x][1] are z coordinates
const float kFrogStartingPositions[3][2] = 
{ 
	{-10.0f, 15.0f},
	{0, 15.0f},
	{10.0f, 15.0f} 
};

void main()
{
	// Create a 3D engine (using TLX engine here) and open a window for it
	myEngine = New3DEngine( kTLX );
	myEngine->StartWindowed();

	// Add default folder for meshes and other media
	myEngine->AddMediaFolder("./Media");

	IFont* myFont = myEngine->LoadFont("Font1.bmp"); //The loading in of fonts so I could display the score
	IFont* myFont1 = myEngine->LoadFont("Font1.bmp"); //The loading in of fonts so I could display the timer
	IFont* gameOver = myEngine->LoadFont("Font1.bmp"); //the loading in of fonts so I could display the game over screen

	//Have to give the IMesh/IModel variables some sort of value otherwise they do not work in a function because their value is null
	//Using visual studio 2019 allows me to not have the IModels and IMeshes initialised to any value while any other version would crash due to null values
	//The meshes store the values they get from the .x file while the models are containers that hold the derivatives from those meshes
	IMesh* skyboxMesh = 0; 
	IModel* skybox = 0;
	IMesh* surfaceMesh = 0;
	IModel* surface = 0;
	IMesh* island1Mesh = 0;
	IModel* island1 = 0;
	IMesh* frogMesh = 0;
	IModel* frog1 = 0;
	IModel* frog2 = 0;
	IModel* frog3 = 0;
	IMesh* transitMesh = 0;
	IMesh* roverMesh = 0;
	IMesh* dummyMesh = 0;
	IModel* dummy = 0;
	IMesh* island2Mesh = 0;
	IModel* island2 = 0;
	IMesh* tyreMesh = 0;


	//To store an array within an array I need two pointers when specifying what data type I am using, presumably
	//this is because I am going through the first array with the first pointer and the second array with the second one
	//I ended up reming the array within an array since it wasn't neccessary or any bit helpful
	IModel* frogs[3] = {frog1, frog2, frog3}; //An array used to store the frogs for easy cycling
	Transit transits[3] = {transit1, transit2, transit3}; //An array used to store the transits for easy cycling
	Rover rovers[3] = {rover1, rover2, rover3}; //An array used to store the rovers for easy cycling
	Tyre tyres[4] = {tyre1, tyre2, tyre3, tyre4}; //An array used to store tyres for easy cycling

	//Where the camera is initialised and what kind of camera it is
	ICamera* myCamera = myEngine->CreateCamera(kManual, 0, kCameraPositionY, kCameraPositionZ);
	myCamera->RotateLocalX(20);

	//Me using my load in models function to create all of the oferementioned models
	LoadInModel(skyboxMesh, skybox, kSkyboxName, 0, kSkyboxPosY, 0); //Creating the skybox
	LoadInModel(surfaceMesh, surface, kSurfaceName, 0, kSurfacePosY, 0); //Creating the water
	LoadInModel(island1Mesh, island1, kIsland1Name, 0, kIsland1PosY, kIsland1PosZ); //creating the first island
	LoadInModel(roverMesh, mixedRover1.rover, kRoverName, kCarStartingPositionX[0], 0, kMixedPosZ); //creating the mixed rover, this can't be done in a loop of regular rovers without issues down the line
	LoadInModel(transitMesh, mixedTransit1.transit, kTransitName, kCarStartingPositionX[1], 0, kMixedPosZ); //creating the mixed transit, this can't be done in a loop of regular transits without issues down the line
	LoadInModel(dummyMesh, dummy, kDummyName, 0, 0, 0); //Used to create the dummy to which the camera and frog will be attached to and which will attach to tyres
	LoadInModel(island2Mesh, island2, kIsland2Name, 0, kIsland2PosY, kIsland2PosZ); //Creating the second island

	//A loop used to create three different models with one loop since they all need the same amount created
	for (int i = 0; i < 3; i++)
	{
		LoadInModel(frogMesh, frogs[i], kFrogName, kFrogStartingPositions[i][0], 0, kFrogStartingPositions[i][1]); //Creating the frogs where the index points at which part of the array to take starting positions from
		LoadInModel(transitMesh, transits[i].transit, kTransitName, kCarStartingPositionX[i], 0, kTransitPosZ); //Creating the transits where the index points at which part of the array to take starting positions from
		LoadInModel(roverMesh, rovers[i].rover, kRoverName, -kCarStartingPositionX[i], 0, kRoverPosZ); //Creating the rovers where the index points at which part of the array to take starting positions from
	}

	//A loop used to create four tyres, looping this means I can cut out 6 lines
	for (int i = 0; i < 4; i++)
	{
		LoadInModel(tyreMesh, tyres[i].tyre, kTyreName, kTyrePosX[i], kTyrePosY, kTyrePosZ[i]); //Creating many tyres where the index shows where to from the array to get the starting positions
		tyres[i].tyre->Scale(kScale); //Scaling up the tyres since they start 10x smaller than they should
	}

	//A loop to rotate the transits and rovers, since the arrays both have 3 of each I can use the same loop for both
	for (int i = 0; i < 3; i++)
	{
		transits[i].transit->RotateLocalY(kRightSideRotation); //Rotating the transits into the correct position where the index selects which transit from the array to change
		rovers[i].rover->RotateLocalY(kLeftSideRotation); //Rotating the rovers into the correct position where the index selects which rover from the array to change
	}

	//Can't use a loop for the mixed cars and a loop would be wasteful so they both get rotated on their own
	mixedRover1.rover->RotateLocalY(kRightSideRotation); //rotates the mixed cars 
	mixedTransit1.transit->RotateLocalY(kRightSideRotation); //rotates the mixed cars into the right position

	int currentFrog = 0; //A variable that I use to check which frog I am currently on

	frogStates frogStateList[3] = {frog1State, frog2State, frog3State}; //An array of the types of states the frogs can be in
	frogDirections frogDirectionList[3] = {frog1Direction, frog2Direction, frog3Direction}; //An array of the directions that the frogs can be in

	float carX = 5; //The size of the car that needs to be deducted from the car's current position to get it's minimum X for the hitbox and added to get the maximum
	float carZ = 5;//The size of the car that needs to be deducted from the car's current position to get it's minimum Z for the hitbox and added to get the maximum
	int lengthOfTransitArray = 3; //The length of the array that I can increase whenever to both move and check collision for
	int lengthOfRoverArray = 3; //The length of the array that I can increase whenever to both move and check collision for
	int carsInLane2and3 = 3; //How many cars are in both of the lanes, allows me to use one less loop if the lengths are the same
	int carsInMixedLane = 2; //How many cars are in the mixed lane and how many need to be moved/how many need their collision checked

	float frogQuarterTurn = 90.0f; //How  many degrees a frog turns if it's not turning into the opposite direction
	float frogHalfTurn = 180.0f; //How many degrees a frog turns if it's turning into the opposite direction

	float limboPosition = 500; //The position to which the cars that are off of the island and in limbo are set

	int zMinBound = 15; //The bound for moving back with the frog to prevent the player from walking off, island
	int xBound = 50; //The bound for moving left and right with the frog to prevent the player from walking off the island, the positive is used for the right the negative for the left since both sides are equal length

	//The minimum and maximum values that are used to store the current positions of a car and then to check if a collision happened
	float carMinX = 0; 
	float carMaxX = 0;
	float carMinZ = 0;
	float carMaxZ = 0;

	myCamera->AttachToParent(dummy); //Attaching the camera to the dummy so it would follow it
	frogs[currentFrog]->AttachToParent(dummy); //attaching the frog to the dummy so it would follow it

	float rotatedLocalAngleX = 0; //A variable used to store how much the camera has moved, the camera is then moved by the negative amount of this to reset it back
	float cameraMovementZMultiplier = 1.3f; //How fast the camera moves into the Z axis, higher than the others for a better visual feel

	float tyreCollisionDetectionRange = 70.0f; //The range after which the tyre collision starts being check, before this range only the cars have their collision checked, this saves a chunk of processing

	float distanceMoved = 0; //How far the frog has moved so far
	bool frogIsMovingSideways = false; //Checking if the frog should move on the X axis or Y axis
	bool frogIsMoving = false; //Checking if the frog is moving, collision detection doesn't happen if it is moving, saves resources, makes it more organic feeling as a jump
	bool movingTheFrog = false; //Checks if the player is allowed to have another key input

	int score = 0; //The score
	int scoreForMoving = 10; //How many points you get for moving forwards once

	float second = 1; //A second, used for a timer with frametime
	int countdown = kBaseCountdown; //The variable that is used to show how much time the player has before their frog dies

	bool collisionWithTyre = false; //Checking if there is any collision with a tyre, for killing the frog or attach it purposes

	int currentTyreCounter = 0; //On which tyre the frog currently resides on

	bool tyreMovingRight = true; //A boolean to check if the tyre should move positively or negatively

	bool frogIsSafe = false; //Used to see which skin the frog should get

	int safeFrogs = 0; //how many frogs are safe

	float kDistanceToMove = 10.0f; //How much the frog should move in total

	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		// Draw the scene
		myEngine->DrawScene();
		float frametime = myEngine->Timer();
		/**** Update your scene each frame here ****/

		if (currentGameState != Paused && currentGameState != Over) //Only allow for the game to be played when the gay is not in a paused or over state, otherwise just draw the score, game over and timer
		{
			//Code used for moving the tyres
			for (int i = 0; i < 4; i++)
			{
				if (tyres[i].MovingRight) //if the tyres are moving right
				{
					tyres[i].tyre->MoveX(kGameSpeed * frametime * (kTyreSpeed + i)); //The i allows me to have all tyres at different speeds
				}
				if (!tyres[i].MovingRight) //if the tyres aren't moving right
				{
					tyres[i].tyre->MoveX(-kGameSpeed * frametime * (kTyreSpeed + i)); //The i allows me to have all tyres at different speeds
				}
				if (tyres[i].tyre->GetX() > xBound)
				{
					tyres[i].MovingRight = false; //Change the state if the tyres are too far on one side
				}
				if (tyres[i].tyre->GetX() < -xBound)
				{
					tyres[i].MovingRight = true; //Change the state if the tyres are too far on one side
				}
			}

			//What I used to move the models and also to put them out of the game for two seconds and then back in
			for (int i = 0; i < carsInLane2and3; i++)
			{
				if (transits[i].timerActive == false)
				{
					MoveCarModels(transits[i].transit, -kTransitSpeedMultiplier, frametime); //The index here selects which car from the array needs to be moved
				}
				TransitLimbo(transits[i], frametime, kResetZone, limboPosition); //Check if the cars should be put into limbo

				if (rovers[i].timerActive == false)
				{
					MoveCarModels(rovers[i].rover, kRoverSpeedMultiplier, frametime); //The index here selects which car from the array needs to be moved
				}
				RoverLimbo(rovers[i], frametime, kResetZone, limboPosition);  //Check if the cars should be put into limbo
			}
			if (mixedTransit1.timerActive == false)
			{
				MoveCarModels(mixedTransit1.transit, -kMixedCarSpeedMultiplier, frametime); //The index here selects which car from the array needs to be moved
			}
			TransitLimbo(mixedTransit1, frametime, kResetZone, limboPosition);  //Check if the cars should be put into limbo

			if (mixedRover1.timerActive == false)
			{
				MoveCarModels(mixedRover1.rover, -kMixedCarSpeedMultiplier, frametime); //The index here selects which car from the array needs to be moved
			}
			MixedRoverLimbo(mixedRover1, frametime, kResetZone, limboPosition);  //Check if the cars should be put into limbo
			//I was using sizeof(cars) / sizeof(cars[0]) because if you just use sizeof(cars[0]) it returns the size of it
			//in bytes rather than the actual length of array, you have to divide it by the byte value of either of the arrays to get the length
			//I decided to scrap this idea since it was counter productive to my original idea of making it scalable since a single integer is easier to use than
			//doing calculations and adding +1 when I add more to the array
			if (frogs[currentFrog]->GetX() <= tyreCollisionDetectionRange) //Decided to use these ifs to save on proccessing, only needs to do one bool check
			{															   //instead of multiple loops that will not matter and will reduce performance
				for (int i = 0; i < lengthOfTransitArray; i++)
				{
					if (currentFrog <= kCurrentFrogLimit)
					{
						GetCoordinates(transits[i].transit, carX, carZ, carMinX, carMaxX, carMinZ, carMaxZ); //Obtain the current hitbox coordinates for the select car
						CheckCollision(currentFrog, frogs[currentFrog], frogs[currentFrog + 1], carMinX, carMaxX, carMinZ, carMaxZ, dummy, distanceMoved, frogIsMoving, frogIsMovingSideways, movingTheFrog, countdown, frogIsSafe); //Check if the frog is in the collision box of the cars in the loop
					}
				}
				for (int i = 0; i < lengthOfRoverArray; i++)
				{
					if (currentFrog <= kCurrentFrogLimit)
					{
						GetCoordinates(rovers[i].rover, carX, carZ, carMinX, carMaxX, carMinZ, carMaxZ); //Obtain the current hitbox coordinates for the select car
						CheckCollision(currentFrog, frogs[currentFrog], frogs[currentFrog + 1], carMinX, carMaxX, carMinZ, carMaxZ, dummy, distanceMoved, frogIsMoving, frogIsMovingSideways, movingTheFrog, countdown, frogIsSafe); //Check if the frog is in the collision box of the cars in the loop
					}
				}
			}
			if (currentFrog <= kCurrentFrogLimit)
			{
				GetCoordinates(mixedTransit1.transit, carX, carZ, carMinX, carMaxX, carMinZ, carMaxZ); //Obtain the current hitbox coordinates for the select car
				CheckCollision(currentFrog, frogs[currentFrog], frogs[currentFrog + 1], carMinX, carMaxX, carMinZ, carMaxZ, dummy, distanceMoved, frogIsMoving, frogIsMovingSideways, movingTheFrog, countdown, frogIsSafe); //Check if the frog is in the collision box of the cars

				GetCoordinates(mixedRover1.rover, carX, carZ, carMinX, carMaxX, carMinZ, carMaxZ); //Obtain the current hitbox coordinates for the select car
				CheckCollision(currentFrog, frogs[currentFrog], frogs[currentFrog + 1], carMinX, carMaxX, carMinZ, carMaxZ, dummy, distanceMoved, frogIsMoving, frogIsMovingSideways, movingTheFrog, countdown, frogIsSafe); //Check if the frog is in the collision box of the cars in the loop
			}
		}

		//:))))))))))))))))))))))))))))))))))))))))))))
		if (currentFrog <= kCurrentFrogLimit)
		{
			if (myEngine->KeyHit(Key_O))
			{
				island1->SetSkin(kFunnyHaha);
			}

			if (myEngine->KeyHit(kPauseKey))
			{
				if (currentGameState == Paused) 
				{
					currentGameState = Playing; //Change the game state back 
				}
				else if (currentGameState != Over && currentGameState == Playing)
				{
					currentGameState = Paused; //Change the game state to paused
				}
			}
			if (!frogIsMoving) //Only do this if the frog has stopped
			{
				if (myEngine->KeyHit(kMoveForward)) //whne the forward key is hit
				{
					score += scoreForMoving; //increase score here since you can only put yourself into dangerous situations when pressing forward so there is no issue awarding the points now
					if (frogDirectionList[currentFrog] != Forward) //If the frog isn't already looking forward continue
					{
						if (frogDirectionList[currentFrog] == Right) //If it's looking right
						{
							frogs[currentFrog]->RotateY(-frogQuarterTurn); //rotate it by -90 degrees
						}
						if (frogDirectionList[currentFrog] == Backwards) //if it's looking back 
						{
							frogs[currentFrog]->RotateY(frogHalfTurn); //do a full turn
						}
						if (frogDirectionList[currentFrog] == Left) //if it's looking left
						{
							frogs[currentFrog]->RotateY(frogQuarterTurn); //rotate it by a half turn
						}
						frogDirectionList[currentFrog] = Forward; //set it's current position to the place it is about to move to
					}
					movingTheFrog = true; //if it's true go down and start moving the frog in whichever direction it is facing
				}
				else if (myEngine->KeyHit(kMoveBackwards))
				{
					if (frogDirectionList[currentFrog] != Backwards)//If the frog isn't already looking backwards continue
					{
						dummy->DetachFromParent();
						if (frogDirectionList[currentFrog] == Right) //If it's looking right
						{
							frogs[currentFrog]->RotateY(frogQuarterTurn);  //rotate it by 90 degrees
						}
						if (frogDirectionList[currentFrog] == Forward) //If it's looking forward
						{
							frogs[currentFrog]->RotateY(frogHalfTurn); //Do a full turn
						}
						if (frogDirectionList[currentFrog] == Left) //If it's looking left
						{
							frogs[currentFrog]->RotateY(-frogQuarterTurn); //Do a -half turn
						}
						frogDirectionList[currentFrog] = Backwards;
					}
					movingTheFrog = true; //if it's true go down and start moving the frog in whichever direction it is facing
				}
				else if (myEngine->KeyHit(kMoveLeft)) //when Z is pressed while frog is idle
				{
					if (frogDirectionList[currentFrog] != Left) //if it's not left continue
					{
						if (frogDirectionList[currentFrog] == Forward) //if it's looking forward
						{
							frogs[currentFrog]->RotateY(-frogQuarterTurn); //do a negative half turn
						}
						if (frogDirectionList[currentFrog] == Backwards) //if it's looking back 
						{
							frogs[currentFrog]->RotateY(frogQuarterTurn); //do a positive half turn
						}
						if (frogDirectionList[currentFrog] == Right) //If it's looking right
						{
							frogs[currentFrog]->RotateY(frogHalfTurn);  //rotate it by 90 degrees
						}
						frogDirectionList[currentFrog] = Left; //set it's current looking position
					}
					movingTheFrog = true; //if it's true go down and start moving the frog in whichever direction it is facing
				}
				else if (myEngine->KeyHit(kMoveRight)) //when X is pressed
				{
					if (frogDirectionList[currentFrog] != Right) //if it's not looking right continue
					{
						if (frogDirectionList[currentFrog] == Forward) //if it's looking forward
						{
							frogs[currentFrog]->RotateY(frogQuarterTurn); //do a half turn
						}
						if (frogDirectionList[currentFrog] == Backwards) //if it's looking back 
						{
							frogs[currentFrog]->RotateY(-frogQuarterTurn); //do a negative half turn
						}
						if (frogDirectionList[currentFrog] == Left) //if it's looking left
						{
							frogs[currentFrog]->RotateY(frogHalfTurn); //do a full turn
						}
						frogDirectionList[currentFrog] = Right; //set it's orientation
					}
					movingTheFrog = true; //if it's true go down and start moving the frog in whichever direction it is facing
				}
			}
			if (movingTheFrog) //if any of the buttons were pressed and moving the frog was set to true
			{
				if (frogDirectionList[currentFrog] == Forward) //if the state is moving forward 
				{
					if (!frogIsMoving) //if the frog isn't moving
					{
						frogIsMoving = true; //frog is moving
					}
					FrogMovement(dummy, frametime, frogIsMovingSideways, distanceMoved, frogIsMoving, kDistanceToMove, kGameSpeed, movingTheFrog, kFrogSpeedMultiplier); //Move the frog using the function
					if (collisionWithTyre) //if it's moving while on a tire
					{
						DetachFromTyre(dummy, tyres[currentTyreCounter - 1].tyre, kFrogStartingPositions[currentFrog][0], kTyrePosY, kFrogStartingPositions[currentFrog][1], collisionWithTyre, currentTyreCounter, frogDirectionList[currentFrog]); //detatch it and allow it to move freely
					}
				}
				if (frogDirectionList[currentFrog] == Backwards)
				{
					if (!frogIsMoving)
					{
						frogIsMoving = true;//frog is moving
					}
					if (frogs[currentFrog]->GetZ() > zMinBound) //if it's not out of bounds
					{
						FrogMovement(dummy, frametime, frogIsMovingSideways, distanceMoved, frogIsMoving, kDistanceToMove, -kGameSpeed, movingTheFrog, kFrogSpeedMultiplier); //Move the frog using the function
						if (collisionWithTyre) //if it's moving while on a tyre
						{
							DetachFromTyre(dummy, tyres[currentTyreCounter - 1].tyre, kFrogStartingPositions[currentFrog][0], kTyrePosY, kFrogStartingPositions[currentFrog][1], collisionWithTyre, currentTyreCounter, frogDirectionList[currentFrog]); //detatch it and allow it to move freely
						}
					}
					else //if the frog can't move back  because it'd move out of bounds
					{
						distanceMoved = 0; //reset distance
						frogIsMoving = false; //reset moving
						frogIsMovingSideways = false; //reset sideways moving
						movingTheFrog = false; //reset player control
					}
				}
				if (frogDirectionList[currentFrog] == Left)
				{
					if (!frogIsMoving) 
					{
						frogIsMoving = true; //frog is now being moved if it wasn't before
					}
					if (frogs[currentFrog]->GetX() >= -xBound) //if it's not out of bounds
					{
						if (!frogIsMovingSideways) //if it wasn't already moving to the sides
						{
							frogIsMovingSideways = true;  //it is now
						}
						FrogMovement(dummy, frametime, frogIsMovingSideways, distanceMoved, frogIsMoving, kDistanceToMove, -kGameSpeed, movingTheFrog, kFrogSpeedMultiplier); //move the frog using the function
						if (collisionWithTyre) //if the frog is in collision with a tyre already
						{
							DetachFromTyre(dummy, tyres[currentTyreCounter - 1].tyre, kFrogStartingPositions[currentFrog][0], kTyrePosY, kFrogStartingPositions[currentFrog][1], collisionWithTyre, currentTyreCounter, frogDirectionList[currentFrog]); //detatch it and allow it to move freely
						}
					}
					else //if it's out of the bounding box
					{
						distanceMoved = 0; //reset how far it moved
						frogIsMoving = false; //reset if it's moving
						frogIsMovingSideways = false; //reset if it's moving sideways
						movingTheFrog = false; //reset player control
					}
				}
				if (frogDirectionList[currentFrog] == Right)
				{
					if (!frogIsMoving) //if the frog wasn't being moved before
					{
						frogIsMoving = true; //it is now 
					}
					if (frogs[currentFrog]->GetX() <= xBound) //if the frog isn't out the bounding box
					{
						if (!frogIsMovingSideways) //if it wasn't already moving sideways
						{
							frogIsMovingSideways = true; //it is now
						}
						FrogMovement(dummy, frametime, frogIsMovingSideways, distanceMoved, frogIsMoving, kDistanceToMove, kGameSpeed, movingTheFrog, kFrogSpeedMultiplier); //Move the frog using the function
						if (collisionWithTyre) //if it's in collision with the tyre
						{
							DetachFromTyre(dummy, tyres[currentTyreCounter - 1].tyre, kFrogStartingPositions[currentFrog][0], kTyrePosY, kFrogStartingPositions[currentFrog][1], collisionWithTyre, currentTyreCounter, frogDirectionList[currentFrog]); //detatch it and allow it to move freely
						}
					}
					else
					{
						distanceMoved = 0; //reset everything back to default
						frogIsMoving = false;
						frogIsMovingSideways = false;
						movingTheFrog = false;
					}
				}
			}

			if (frogs[currentFrog]->GetZ() >= tyreCollisionDetectionRange && collisionWithTyre == false && frogIsMoving == false) //if the frog is in tyre collision range check for collision, saves resources
			{
				for (int i = 0; i < 4; i++)
				{
					CheckTyreCollision(frogs[currentFrog], tyres[i].tyre, collisionWithTyre, dummy, kFrogStartingPositions[currentFrog][0], kFrogStartingPositions[currentFrog][1], kTyrePosY, frogDirectionList[currentFrog], currentTyreCounter); //check the sphere to sphere collision between frogs with the function
					if (collisionWithTyre == true)
					{
						break; //if it collided with any of the tyres break out of the loop, don't need to check every single tyre if the first or second or third had a collision
					}
				}
			}
			if (frogs[currentFrog]->GetZ() > kMinWater && collisionWithTyre == false && frogIsMoving == false && frogs[currentFrog]->GetZ() < kMaxWater) //Check if the frog has drowned, only after it's in range of the water, saves resources
			{
				countdown = kBaseCountdown; //reset the countdown when frog dies
				FrogRipped(currentFrog, frogs[currentFrog], frogs[currentFrog + 1], dummy, distanceMoved, frogIsMoving, frogIsMovingSideways, movingTheFrog, frogIsSafe); //using the function kill off the frog and move onto the next one
				currentTyreCounter = 0; //reset which tyre the previous frog was on, if it was on any
			}
			if (frogs[currentFrog]->GetZ() > kMaxWater && collisionWithTyre == false && frogIsMoving == false) //if the player gets the frogs accross the water
			{
				safeFrogs++; //increase how many frogs are safe
				frogIsSafe = true; //set frog to safe, used for skin changing
				FrogRipped(currentFrog, frogs[currentFrog], frogs[currentFrog + 1], dummy, distanceMoved, frogIsMoving, frogIsMovingSideways, movingTheFrog, frogIsSafe); //kill off the current frog but this time use a different skin since it's safe
				currentTyreCounter = 0; //reset tyre counter
				countdown = kBaseCountdown; //reset kill frog countdown
			}

			if (second > 0) 
			{
				second -= frametime; //a scuffed version of a countdown, each time you return a frametime, minus it from one second for as long as it's more than 0
			}
			else
			{
				second = 1; //reset second
				countdown -= second; //remove a second off of the timer
				if (countdown < 0)
				{
					countdown = kBaseCountdown; //reset the countdown if it's less than 0
					if (collisionWithTyre)
					{
						DetachFromTyre(dummy, tyres[currentTyreCounter - 1].tyre, kFrogStartingPositions[currentFrog][0], kTyrePosY, kFrogStartingPositions[currentFrog][1], collisionWithTyre, currentTyreCounter, frogDirectionList[currentFrog]); //if the frog is on a tyre, remove it off of it
						currentTyreCounter = 0; //reset the counter of the tyres
					}
					FrogRipped(currentFrog, frogs[currentFrog], frogs[currentFrog + 1], dummy, distanceMoved, frogIsMoving, frogIsMovingSideways, movingTheFrog, frogIsSafe); //kill off the frog and move onto the next one
				}
			}


			if (myEngine->KeyHit(kResetCamera)) 
			{
				myCamera->RotateLocalX(-rotatedLocalAngleX); //rotate the camera localX back by however much it was moved by
				myCamera->MoveZ(-rotatedLocalAngleX * cameraMovementZMultiplier); //move the camera Z back by howeverm uch it was moved by
				myCamera->MoveY(-rotatedLocalAngleX); //move the camera Y by however much it moved
				rotatedLocalAngleX = 0; //set how much it was moved by back to 0
			}
			else if (myEngine->KeyHeld(kRotateUp))
			{
				if (kCameraPositiveCap > rotatedLocalAngleX) //if it isn't above the cap
				{
					myCamera->RotateLocalX(kGameSpeed * frametime); //rotate the camera each frame
					rotatedLocalAngleX += kGameSpeed * frametime; //store how much it moved by this frame
					myCamera->MoveZ(kGameSpeed * frametime * cameraMovementZMultiplier); //move the camera z each frame multiplied by the camera speed multiplier to make it move faster
					myCamera->MoveY(kGameSpeed * frametime); //move the camera each frame
				}
			}
			else if (myEngine->KeyHeld(kRotateDown))
			{
				if (kCameraNegativeCap < rotatedLocalAngleX)
				{
					myCamera->RotateLocalX(-kGameSpeed * frametime); //rotate the camera each frame
					rotatedLocalAngleX += -kGameSpeed * frametime; // store how much it moved by this frame
					myCamera->MoveZ(-kGameSpeed * frametime * cameraMovementZMultiplier);//move the camera z each frame multiplied by the camera speed multiplier to make it move faster
					myCamera->MoveY(-kGameSpeed * frametime); //move the camera each frame
				}
			}
		}
	if (myEngine->KeyHit(kExitKey))
	{
		myEngine->Stop(); //quit out the game
	}
	myFont1->Draw("Timer: " + to_string(countdown), kTextPosition2, kTextPosition1); //draw the timer
	myFont->Draw("Your score is: " + to_string(score), kTextPosition1, kTextPosition1); //draw the score
	if (currentGameState == Over)
	{
		gameOver->Draw("Game Over", kTextOverX, kTextOverY); //if the game is over then draw the game over text
	}
	}
	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}

//I use frogripped to reset absolutely everything that the frogs can impact back to their base values
void FrogRipped(int& currentFrogNumber, IModel*& currentFrog, IModel*& nextFrog, IModel*& dummy, float& distanceMoved, bool& frogIsMoving, bool& frogIsMovingSideways, bool& movingTheFrog, bool& frogIsSafe)
{
	float dummyPositionX = dummy->GetX() + kFrogStartingPositions[currentFrogNumber][0]; //store the dead frog position value that depends on the dummy position
	float dummyPositionZ = dummy->GetZ() + kFrogStartingPositions[currentFrogNumber][1]; //store the dead frog position value that depends on the dummy position
	if (frogIsSafe == false)
	{
		currentFrog->SetSkin(kDeadImageName); //Change the frog skin if the frog died
	}
	else if (frogIsSafe == true)
	{
		currentFrog->SetSkin(kSafeImageName); //change the frog skin if the frog is safe
	}
	if (frogIsSafe == true)
	{
		frogIsSafe = false; //reset the frog safety
	}
	currentFrog->Scale(deadFrogScale); //scale the dead frog to 0.4 to make it smalled
	currentFrog->DetachFromParent(); //detach it from the parent if it was on any
	currentFrog->SetX(dummyPositionX); //what position the dead frog should be at
	currentFrog->SetZ(dummyPositionZ); //what position the dead frog should be at 
	currentFrogNumber++; //increase current frog so it can move onto the next one
	dummy->SetX(0); //move dummy position to default
	dummy->SetZ(0); //move dummy position to default 
	if (currentFrogNumber > 2)
	{
		currentGameState = Over; //set game to over if all three frogs dead
	}
	else
	{
		nextFrog->AttachToParent(dummy); //otherwise attach the dummy tothe next frog
	}
	distanceMoved = 0; //reset distance moved
	frogIsMoving = false; //reset if the frog is moving
	frogIsMovingSideways = false; //reset if it was moving sideways
	movingTheFrog = false; //reset player controls
}

//used to load in models, it may be wasteful to load in a mesh each time but I was assured by a tutor that it is not too bad
void LoadInModel(IMesh* meshName, IModel*& modelName, string loadMeshName, float modelPosX, float modelPosY, float modelPosZ)
{
	meshName = myEngine->LoadMesh(loadMeshName); //load in the .x file
	modelName = meshName->CreateModel(modelPosX, modelPosY, modelPosZ); //give the properties of the .x file to an object in the game
}

//moving the cars, fairly siple
void MoveCarModels(IModel*& modelName, float speedMultiplier, float frametime)
{
	modelName->MoveX(kGameSpeed * frametime * speedMultiplier); //move the cars by the game speed multiplied by frametime and speed multi each frame
}

//check the collision for cars, I was foolish and decided to put my frog is dead function inside of another function, this was a terrible idea, never again
void CheckCollision(int& currentFrogNumber, IModel*& currentFrog, IModel*& nextFrog, float& carMinX, float& carMaxX, float& carMinZ, float& carMaxZ, IModel*& dummy, float& distanceMoved, bool& frogIsMoving, bool& frogIsMovingSideways, bool& movingTheFrog, int& countdowntimer, bool& frogIsSafe)
{
	float x = currentFrog->GetX(); //get the current position of the frog x for point to box checks
	float z = currentFrog->GetZ(); //get the current position of the frog z for point to box checks
	if (x > carMinX && x < carMaxX && z > carMinZ && z < carMaxZ)
	{
		FrogRipped(currentFrogNumber, currentFrog, nextFrog, dummy, distanceMoved, frogIsMoving, frogIsMovingSideways, movingTheFrog, frogIsSafe); //if the numbers match up, kill the frog and move on, agian, terrible idea to do this inside of another function
		countdowntimer = kBaseCountdown; //reset the countdown back to 20
	}

}

//check where the cars actually are, used in the check collision function
void GetCoordinates(IModel*& car, float carX, float carZ, float& carMinX, float& carMaxX, float& carMinZ, float& carMaxZ)
{
	carMinX = car->GetX() - carX - kFrogRadius; //set the minimum of the hitbox x
	carMaxX = car->GetX() + carX + kFrogRadius; //set the maximum of the hitbox x
	carMinZ = car->GetZ() - carZ - kFrogRadius; //set the minimum of the hitbox z
	carMaxZ = car->GetZ() + carZ + kFrogRadius; //set the maximum of the hitbox z
}

//limbo refers to where the cars go when they are not on the islands, this is used to teleport transits off screen
void TransitLimbo(Transit& transit, float frametime, float kResetZone, float limboPosition)
{
	if (transit.timerActive == true)
	{
		transit.timer -= frametime; //count down the two seconds if they are in limboi
	}
	if (transit.transit->GetX() <= -kResetZone)
	{
		if (transit.timerActive == false) 
		{
			transit.timerActive = true; //if the car wasn't already in limbo but now has met the criteria, put it into limbo
			transit.transit->SetX(-limboPosition); //set it's position to be off screen
		}
		if (transit.timer <= 0)
		{
			transit.timerActive = false; //once the timer runs out reset the obol
			transit.transit->SetX(kResetZone); //set the car position to the starting zone
			transit.timer = 2; //reset the two seconds
		}
	}
}

//limbo refers to where the cars go when they are not on the islands, this is used to teleport rovers off screen
void RoverLimbo(Rover& rover, float frametime, float kResetZone, float limboPosition)
{
	if (rover.timerActive == true)
	{
		rover.timer -= frametime;  //count down the two seconds if they are in limbo
	}
	if (rover.rover->GetX() >= kResetZone)
	{
		if (rover.timerActive == false)
		{
			rover.timerActive = true; //if the car wasn't already in limbo but now has met the criteria, put it into limbo
			rover.rover->SetX(limboPosition); //set it's position to be off screen
		}
		if (rover.timer <= 0)
		{
			rover.timerActive = false; //once the timer runs out reset the obol
			rover.rover->SetX(-kResetZone); //set the car position to the starting zone
			rover.timer = 2; //reset time
		}
	}
}

//A limbo for the mixed cars, it needs it's own thing otherwise things go bad
void MixedRoverLimbo(Rover& rover, float frametime, float kResetZone, float limboPosition)
{
	if (rover.timerActive == true)
	{
		rover.timer -= frametime; //count down the two seconds if they are in limbo
	}
	if (rover.rover->GetX() <= -kResetZone) 
	{
		if (rover.timerActive == false)
		{
			rover.timerActive = true; //if the car wasn't already in limbo but now has met the criteria, put it into limbo
			rover.rover->SetX(-limboPosition); //set it's position to be off screen
		}
		if (rover.timer <= 0)
		{
			rover.timerActive = false; //once the timer runs out reset the bool
			rover.rover->SetX(kResetZone); //set the car position to the starting zone
			rover.timer = 2; //reset time
		}
	}
}

//Check for the frog collision with the tyres using sphere to sphere
void CheckTyreCollision(IModel* frogModel, IModel* tyreModel, bool& collisionWithTyre, IModel*& dummy, float kFrogStartingPositionsX, float kFrogStartingPositionsZ, float  kTyrePosY, frogDirections frogDirection, int& currentTyreCounter)
{
	float x, z, collisionDist; //create storage variables
	x = frogModel->GetX() - tyreModel->GetX(); //store the difference in x coordinates
	z = frogModel->GetZ() - tyreModel->GetZ(); //store the difference in y coordinates
	collisionDist = sqrt(x*x + z*z); //calculate the distance between the two spheres
	if (collisionDist < kTyreRadius)  //if the radius is smaller than the distance, collision occured
	{
		if (frogDirection == Forward)
		{
			currentTyreCounter++; //if the player moves forward and is on the tyres, increase which tyre they're on
		}
		dummy->Scale(0.1f); //set scale since attaching to parent will increase it by 10x
		dummy->AttachToParent(tyreModel); //attach to the tyre the frog is now on
		dummy->SetX(tyreModel->GetX() - kFrogStartingPositionsX); //set the dummy x to the same as the tyre's and minus the extra that the dummy already had
		dummy->SetY(tyreModel->GetY() - kTyrePosY); //set the dummy y to the same as the tyre's and minus the extra that the dummy already had
		dummy->SetZ(tyreModel->GetZ() - kFrogStartingPositionsZ); //set the dummy z to the same as the tyre's and minus the extra that the dummy already had
		if (collisionWithTyre == false)
		{
			collisionWithTyre = true; //if it wasn't already colliding with a tyre, it now will
		}
	}
	else
	{
		collisionWithTyre = false; //if the radius is lower thna the distance set bool to false
	}
}

//the function used to move the actual frog
void FrogMovement(IModel*& dummy, float& frametime, bool& frogIsMovingSideways, float& distanceMoved, bool& frogIsMoving, float& distanceTraveled, float kGameSpeed, bool& movingTheFrog, float kFrogSpeedMultiplier)
{
	if (frogIsMovingSideways)
	{
		dummy->MoveX(frametime * kGameSpeed * kFrogSpeedMultiplier); //if the frog is moving sideways move it on the x axis
	}
	if (!frogIsMovingSideways)
	{
		dummy->MoveZ(frametime * kGameSpeed * kFrogSpeedMultiplier); //if it's not moving sideways move it on the z axis
	}
	distanceMoved += frametime * kGameSpeed * kFrogSpeedMultiplier; //check how far the frog has moved so far
	if (distanceMoved >= distanceTraveled || distanceMoved <= -distanceTraveled) //if the frog has moved 10 units
	{
		distanceMoved = 0; //reset how far it moved
		frogIsMoving = false; //reset if it's moving 
		frogIsMovingSideways = false; //reset if it's moving sideways
		movingTheFrog = false; //reset player control
	}
}

//a function that allows the frog to detach from the tyre it is attached to so it could move to other tyres or into the wtaer
void DetachFromTyre(IModel*& dummy, IModel*& tyreModel, float kFrogStartingPositionsX, float kTyrePosY, float kFrogStartingPositionsZ, bool& collisionWithTyre, int& currentTyreCounter, frogDirections frogDirection)
{
	dummy->DetachFromParent(); //detach it from current tyre it's on 
	dummy->Scale(10); //set scale back to 10x of 0.1
	dummy->SetX(tyreModel->GetX() - kFrogStartingPositionsX); //set the position to the tyre model minus the frogs usual positioning
	dummy->SetY(tyreModel->GetY() - kTyrePosY); //set the position to the tyre model minus the frogs usual positioning
	dummy->SetZ(tyreModel->GetZ() - kFrogStartingPositionsZ); //set the position to the tyre model minus the frogs usual positioning
	collisionWithTyre = false; //set that the frog is no longer colliding with a tyre
	if (frogDirection == Backwards)
	{
		currentTyreCounter--; //if the player moved back reduce which tyre the frog is on by 1
	}
}