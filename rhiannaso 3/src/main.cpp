/*
 * Program 4 example with diffuse and spline camera PRESS 'g'
 * CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn (spline D. McGirr)
 */

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <math.h> 
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "ShapeInst.h"
#include "Keyframe.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Texture.h"
#include "util.h"
#include "stb_image.h"
#include "Bezier.h"
#include "Spline.h"
#include "particleSys.h"
#include "irrKlang.h"
#include "TextGen.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>
#define PI 3.1415927

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll

using namespace std;
using namespace glm;
//using namespace irrklang;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program - use this one for Blinn-Phong has diffuse
	std::shared_ptr<Program> prog;

	//Our shader program for textures
	std::shared_ptr<Program> texProg;

    //Our shader program for skybox
	std::shared_ptr<Program> cubeProg;

    std::shared_ptr<Program> progInst;

    std::shared_ptr<Program> partProg;

    std::shared_ptr<Program> glyphProg;

	//our geometry
	shared_ptr<Shape> sphere;

	shared_ptr<Shape> theDog;

	shared_ptr<Shape> cube;

    shared_ptr<Shape> cubeTex;
    vector<shared_ptr<ShapeInst>> cubeInst;

    shared_ptr<Shape> roadObj;
    shared_ptr<Shape> lampMesh;
    vector<shared_ptr<Shape>> houseMesh;
    vector<shared_ptr<Shape>> carMesh;
    vector<shared_ptr<Shape>> dummyMesh;
    vector<shared_ptr<Shape>> statueMesh2;
    vector<shared_ptr<Shape>> axeMesh;
    vec3 dummyMin;
    vec3 dummyMax;

    vector<tinyobj::material_t> houseMat;
    vector<tinyobj::material_t> carMat;
    vector<tinyobj::material_t> treeMat;
    map<string, shared_ptr<Texture>> textureMap;

    //animation data
	float lightTrans = 0;
    float driveTheta = 0;
    float frameDur = 0.25;
    float startTime = 0;
    bool isNewFrame = true;
    int overallFrame = 0;
    vector<float> limbRot{0, 0, 0, 0, 0, 0};
    // vector<float> lArmKF{0, PI/10.0, PI/8.0, 0, -PI/10.0, -PI/8.0};
    // vector<float> rArmKF{0, PI/10.0, PI/8.0, 0, -PI/10.0, -PI/8.0};
    // vector<float> lLegKF{PI/8.0, PI/10.0, 0, -PI/8.0, -PI/10.0, 0};
    // vector<float> lKneeKF{0, PI/4.0, PI/3.0, 0, 0, 0};
    // vector<float> rLegKF{-PI/8.0, -PI/10.0, 0, PI/8.0, PI/10.0, 0};
    // vector<float> rKneeKF{0, 0, 0, 0, PI/4.0, PI/3.0};
    vector<Keyframe> lArmKF;
    vector<Keyframe> rArmKF;
    vector<Keyframe> lLegKF;
    vector<Keyframe> lKneeKF;
    vector<Keyframe> rLegKF;
    vector<Keyframe> rKneeKF;
    bool isWalking = false;

    // freetext
    FT_Library ft;
    TextGen* writer;
    bool gameOver = false;
    bool lostGame = false;
    bool startScreen = true;
    bool timedGame = false;
    float timer = 300; // 5 minutes
    float gameStart = 0;
    float gameEnd = 0;

    string rDir; // resource directory

    //skybox data
    vector<std::string> faces {
        "right.jpg",
        "left.jpg",
        "top.jpg",
        "bottom.jpg",
        "front.jpg",
        "back.jpg"
    }; 
    unsigned int cubeMapTexture;

	//global data for ground plane - direct load constant defined CPU data to GPU (not obj)
	GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
	int g_GiboLen;
	//ground VAO
	GLuint GroundVertexArrayID;

	//the image to use as a texture (ground)
    shared_ptr<Texture> brick;
    shared_ptr<Texture> blackText;
    shared_ptr<Texture> lightWood;
    shared_ptr<Texture> bumpBrick;
    shared_ptr<Texture> whiteText;
    shared_ptr<Texture> tiles;
    shared_ptr<Texture> stone;
    shared_ptr<Texture> whiteWood;
    shared_ptr<Texture> glass;
    shared_ptr<Texture> dirt;
    shared_ptr<Texture> statueTex2;
    shared_ptr<Texture> spruceLeaf;
    shared_ptr<Texture> spruceTrunk;
    shared_ptr<Texture> axeTex;
    shared_ptr<Texture> texture;

    //the partricle system
	particleSys *thePartSystem;
    vec3 leavesPos = vec3(0, 0, 0); // tmp val

    int occupancy[31][31] = {0};
    vector<float> positions;
    glm::mat4 *modelMatrices;
    vector<vec2> choppedTrees;

	//camera
	double g_phi, g_theta;
    vec3 dummyLoc = vec3(16, -1.25, 27.25);
    float dummyRot = PI/2.0;
	vec3 view = vec3(0, 0, 1);
	vec3 strafe = vec3(1, 0, 0);
    float speed = 0.3;
    float camY = 1.8;
    float camZ = 1.4;
    vec3 g_eye = vec3(-20, 12, -20);
    //vec3 g_eye = vec3(dummyLoc.x, dummyLoc.y+camY, dummyLoc.z+camZ); // 16, 0, 33
    vec3 g_lookAt = vec3(dummyLoc.x, dummyLoc.y+camY, dummyLoc.z); // 16, 0, 30
    vec3 gaze = g_eye - g_lookAt;

    // Chopping actions
    int hasAxe = 0;
    bool firstAct = true;
    float axe_scale[3] = {0.001, 0.001, 0.001};
    float axe_x[3] = {mapSpaces(11, 15).x, mapSpaces(13, 5).x, mapSpaces(27, 17).x};
    float axe_z[3] = {mapSpaces(11, 15).z, mapSpaces(13, 5).z, mapSpaces(27, 17).z};
    bool axe_taken[3] = {false, false, false};
    bool isChopping = false;

	Spline splinepath[2];
	bool goCamera = false;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_Q && action == GLFW_PRESS){
			lightTrans -= 0.5;
		}
		if (key == GLFW_KEY_E && action == GLFW_PRESS){
			lightTrans += 0.5;
		}
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
        if (key == GLFW_KEY_V && action == GLFW_PRESS) { // Show finish
            dummyLoc = vec3(mapSpaces(0, 17).x, -1.25, mapSpaces(0, 17).z);
            g_eye = vec3(dummyLoc.x, dummyLoc.y+camY, dummyLoc.z+camZ);
            g_lookAt = vec3(dummyLoc.x, dummyLoc.y+camY, dummyLoc.z);
            gaze = g_eye - g_lookAt;
            g_theta = -PI/2.0;
            computeLookAt();
		}
        if (key == GLFW_KEY_X && action == GLFW_PRESS) { // Show axe functionality
			dummyLoc = vec3(mapSpaces(27, 16).x, -1.25, mapSpaces(27, 16).z);
            g_eye = vec3(dummyLoc.x, dummyLoc.y+camY, dummyLoc.z+camZ);
            g_lookAt = vec3(dummyLoc.x, dummyLoc.y+camY, dummyLoc.z);
            gaze = g_eye - g_lookAt;
            g_theta = 0;
            computeLookAt();
		}
        if (key == GLFW_KEY_R && action == GLFW_PRESS) { // Restart
            goCamera = false;
            dummyLoc = vec3(16, -1.25, 27.25);
            g_eye = vec3(-20, 12, -20);
            g_lookAt = vec3(dummyLoc.x, dummyLoc.y+camY, dummyLoc.z);
            gaze = g_eye - g_lookAt;
            splinepath[0] = Spline(glm::vec3(-20,12,-20), glm::vec3(-10,10,-10), glm::vec3(0, 8, 0), glm::vec3(10,6,10), 3);
            splinepath[1] = Spline(glm::vec3(10,6,10), glm::vec3(20,4,20), glm::vec3(25, 2, 30), glm::vec3(16,-1.25+camY,27.25+camZ), 3);
            for (int i=0; i < 3; i++) {
                axe_taken[i] = false;
                axe_scale[i] = 0.001;
            }
            isChopping = false;
            readMaze(rDir);
            firstAct = true;
            hasAxe = 0;
            gameOver = false;
            startScreen = true;
            overallFrame = 0;
            for (int k=0; k < choppedTrees.size(); k++) { // Reset any chopped trees
                vec2 spot = choppedTrees[k];
                vec3 tmp = mapSpaces(spot.x, spot.y);
                mat4 t1 = glm::translate(glm::mat4(1.0f), glm::vec3(tmp.x, tmp.y, tmp.z));
                mat4 r = glm::rotate(glm::mat4(1.0f), (float)(-PI/2), vec3(1, 0, 0));
                mat4 s = glm::scale(glm::mat4(1.0f), glm::vec3(0.45));
                modelMatrices[(int)spot.x*31+(int)spot.y] = t1*r*s;
                for (int j=0; j < cubeInst.size(); j++) {
                    cubeInst[j]->update(modelMatrices);
                }
            }
            choppedTrees.clear();
            lostGame = false;
            timer = 300;
            g_phi = 0;
            g_theta = -PI/2;
            dummyRot = PI/2.0;
        }
        if (key == GLFW_KEY_1 && action == GLFW_PRESS) { // Explore game mode
            timedGame = false;
            startScreen = false;
			goCamera = !goCamera;
            if (!goCamera)
                computeLookAt();
		}
        if (key == GLFW_KEY_2 && action == GLFW_PRESS) { // Timed game mode
            timedGame = true;
            startScreen = false;
			goCamera = !goCamera;
            if (!goCamera)
                computeLookAt();
		}
        if (key == GLFW_KEY_T && action == GLFW_PRESS) { // Chopping down trees
            vec2 tmp = findMySpace(dummyLoc);
            if (hasAxe > 0 && firstAct) {
                vec2 chopLoc = findMySpace(g_lookAt);
                float i = chopLoc.x;
                float j = chopLoc.y;
                vec3 view = g_eye-g_lookAt;
                // cout << "VIEW: " << view.x << endl;
                // cout << "VIEW: " << view.z << endl;
                view = vec3(round(view.x), round(view.y), round(view.z));
                mat4 s = glm::scale(glm::mat4(1.0f), glm::vec3(0.0));
                if (view.x > 0) {
                    j -= 1;
                } else if (view.x < 0) {
                    j += 1;
                } else {
                    if (view.z > 0) {
                        i -= 1;
                    } else if (view.z < 0) {
                        i += 1;
                    }
                }
                // cout << i << endl;
                // cout << j << endl;
                if (occupancy[(int)i][(int)j] != 0) { // Only act if facing a tree
                    choppedTrees.push_back(vec2(i, j));
                    leavesPos = vec3(mapSpaces(i, j).x, 0.25, mapSpaces(i, j).z);
                    thePartSystem = new particleSys(leavesPos);
		            thePartSystem->gpuSetup();
                    isChopping = true;
                    occupancy[(int)i][(int)j] = 0; // mark as barrier gone
                    modelMatrices[(int)i*31+(int)j] = s; // redraw with tree gone
                    for (int k=0; k < cubeInst.size(); k++) {
                        cubeInst[k]->update(modelMatrices);
                    }
                    cout << "Chopping" << endl;
                    hasAxe -= 1;
                    firstAct = false;
                }
            }
            if (hasAxe == 0 && firstAct) {
                if (tmp.x == 11 && tmp.y == 15 && !axe_taken[0]) {
                    axe_scale[0] = 0;
                    axe_taken[0] = true;
                    cout << "Got axe" << endl;
                    hasAxe += 1;
                } if (tmp.x == 13 && tmp.y == 5 && !axe_taken[1]) {
                    axe_scale[1] = 0;
                    axe_taken[1] = true;
                    cout << "Got axe" << endl;
                    hasAxe += 1;
                } if (tmp.x == 27 && tmp.y == 17 && !axe_taken[2]) {
                    axe_scale[2] = 0;
                    axe_taken[2] = true;
                    cout << "Got axe" << endl;
                    hasAxe += 1;
                }
                firstAct = false;
            }
            firstAct = true;
		}
		if (key == GLFW_KEY_G && action == GLFW_RELEASE) {
            startScreen = false;
			goCamera = !goCamera;
            if (!goCamera)
                computeLookAt();
		}
        if (key == GLFW_KEY_W && action == GLFW_PRESS){
			view = g_lookAt - g_eye;
            vec3 tmp = dummyLoc + (speed*view);
            if (!detectCollision(tmp) && !lostGame) {
                isWalking = true;
                // dummyLoc = dummyLoc + (speed*view);
                dummyLoc = vec3(tmp.x, -1.25, tmp.z);
                // g_eye = g_eye + (speed*view);
                g_lookAt = vec3(dummyLoc.x, dummyLoc.y+camY, dummyLoc.z);
                computeLookAt();
                // g_lookAt = g_lookAt + (speed*view);
            }
		}
        if (key == GLFW_KEY_A && action == GLFW_PRESS){
            view = g_lookAt - g_eye;
            strafe = cross(view, vec3(0, 1, 0));
            if (!detectCollision(dummyLoc - (speed*strafe)) && !lostGame) {
                isWalking = true;
                dummyLoc = dummyLoc - (speed*strafe);
                g_eye = g_eye - (speed*strafe);
                g_lookAt = vec3(dummyLoc.x, dummyLoc.y+camY, dummyLoc.z);
                // g_lookAt = g_lookAt - (speed*strafe);
            }
		}
        if (key == GLFW_KEY_S && action == GLFW_PRESS){
			view = g_lookAt - g_eye;
            vec3 tmp = dummyLoc - (speed*view);
            if (!detectCollision(tmp) && !lostGame) {
                isWalking = true;
                // dummyLoc = dummyLoc - (speed*view);
                dummyLoc = vec3(tmp.x, -1.25, tmp.z);
                // g_eye = g_eye - (speed*view);
                g_lookAt = vec3(dummyLoc.x, dummyLoc.y+camY, dummyLoc.z);
                computeLookAt();
                // g_lookAt = g_lookAt - (speed*view);
            }
		}
        if (key == GLFW_KEY_D && action == GLFW_PRESS){
            view = g_lookAt - g_eye;
            strafe = cross(view, vec3(0, 1, 0));
            if (!detectCollision(dummyLoc + (speed*strafe)) && !lostGame) {
                isWalking = true;
                dummyLoc = dummyLoc + (speed*strafe);
                g_eye = g_eye + (speed*strafe);
                g_lookAt = vec3(dummyLoc.x, dummyLoc.y+camY, dummyLoc.z);
                // g_lookAt = g_lookAt + (speed*strafe);
            }
		}
	}

    vec2 findMySpace(vec3 myPos) { // x,z to i,j
        float x = round(myPos.x/2) * 2; // round to nearest even (all spaces centered on even #s)
        float z = round(myPos.z/2) * 2;

        float i = (0.5*z) + 15;
        float j = (0.5*x) + 15;
        return vec2(i,j);
    }

    bool detectCollision(vec3 myPos) {
        vec2 occPos = findMySpace(myPos);
        if (occPos.x < 0) {
            gameOver = true;
            gameEnd = glfwGetTime();
            return true;
        }
        if (occPos.x > 30 || occPos.y > 30 || occPos.y < 0) // If beyond the bounds of the maze
            return true;
        if (occupancy[(int)occPos.x][(int)occPos.y] != 1) // If not a wall
            return false;
        else
            return true;
    }

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			 glfwGetCursorPos(window, &posX, &posY);
			 cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
		}
	}


	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY) {
        if (!goCamera && !startScreen) {
            g_theta -= deltaX/100;
            if (g_phi < DegToRad(60) && g_phi > -DegToRad(80)) {
                g_phi += deltaY/100;
            }
            if (g_phi >= DegToRad(60) && deltaY < 0) {
                g_phi += deltaY/100;
            }
            if (g_phi <= DegToRad(80) && deltaY > 0) {
                g_phi += deltaY/100;
            }
            computeLookAt();
        }
	}

    float DegToRad(float degrees) {
        return degrees*(PI/180.0);
    }

    void computeLookAt() {
        // float radius = 1.0;
        float radius = 1.4;
        dummyRot = -g_theta;
        float x = radius*cos(g_phi)*cos(g_theta);
        float y = radius*sin(g_phi);
        float z = radius*cos(g_phi)*cos((PI/2.0)-g_theta);
        g_eye = g_lookAt - vec3(x, y, z);
        gaze = g_eye - g_lookAt;
        // g_lookAt = vec3(x, y, z) + g_eye;
    }

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

    void setTextures(const std::string& resourceDirectory) {
        //read in a load the texture
        brick = make_shared<Texture>();
  		brick->setFilename(resourceDirectory + "/brickHouse/campiangatebrick1.jpg");
  		brick->init();
  		brick->setUnit(0);
  		brick->setWrapModes(GL_REPEAT, GL_REPEAT);
        textureMap.insert(pair<string, shared_ptr<Texture>>("campiangatebrick1.jpg", brick));

        blackText = make_shared<Texture>();
  		blackText->setFilename(resourceDirectory + "/brickHouse/063.jpg");
  		blackText->init();
  		blackText->setUnit(1);
  		blackText->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        textureMap.insert(pair<string, shared_ptr<Texture>>("063.JPG", blackText));

        lightWood = make_shared<Texture>();
  		lightWood->setFilename(resourceDirectory + "/brown_wood.jpg");
  		lightWood->init();
  		lightWood->setUnit(2);
  		lightWood->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

        bumpBrick = make_shared<Texture>();
  		bumpBrick->setFilename(resourceDirectory + "/brickHouse/campiangatebrick1_bump.jpg");
  		bumpBrick->init();
  		bumpBrick->setUnit(3);
  		bumpBrick->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        textureMap.insert(pair<string, shared_ptr<Texture>>("campiangatebrick1_bump.jpg", bumpBrick));

        whiteText = make_shared<Texture>();
  		whiteText->setFilename(resourceDirectory + "/brickHouse/HighBuild_texture.jpg");
  		whiteText->init();
  		whiteText->setUnit(4);
  		whiteText->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        textureMap.insert(pair<string, shared_ptr<Texture>>("HighBuild_texture.jpg", whiteText));

        tiles = make_shared<Texture>();
  		tiles->setFilename(resourceDirectory + "/brickHouse/panTiles_1024_more_red.jpg");
  		tiles->init();
  		tiles->setUnit(5);
  		tiles->setWrapModes(GL_REPEAT, GL_REPEAT);
        textureMap.insert(pair<string, shared_ptr<Texture>>("panTiles_1024_more_red.jpg", tiles));

        stone = make_shared<Texture>();
  		stone->setFilename(resourceDirectory + "/brickHouse/stones006x04.jpg");
  		stone->init();
  		stone->setUnit(6);
  		stone->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        textureMap.insert(pair<string, shared_ptr<Texture>>("stones006x04.jpg", stone));

        whiteWood = make_shared<Texture>();
  		whiteWood->setFilename(resourceDirectory + "/white_wood.jpg");
  		whiteWood->init();
  		whiteWood->setUnit(7);
  		whiteWood->setWrapModes(GL_REPEAT, GL_REPEAT);

        glass = make_shared<Texture>();
  		glass->setFilename(resourceDirectory + "/glass.jpg");
  		glass->init();
  		glass->setUnit(8);
  		glass->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

        dirt = make_shared<Texture>();
  		dirt->setFilename(resourceDirectory + "/ground.jpg");
  		dirt->init();
  		dirt->setUnit(9);
  		dirt->setWrapModes(GL_REPEAT, GL_REPEAT);

        statueTex2 = make_shared<Texture>();
  		statueTex2->setFilename(resourceDirectory + "/statue2/mm_facade_sculpture_03_diffus.jpg");
  		statueTex2->init();
  		statueTex2->setUnit(10);
  		statueTex2->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

        spruceLeaf = make_shared<Texture>();
  		spruceLeaf->setFilename(resourceDirectory + "/Spruce_obj/Spruce_branches.png");
  		spruceLeaf->init();
  		spruceLeaf->setUnit(11);
  		spruceLeaf->setWrapModes(GL_REPEAT, GL_REPEAT);
        textureMap.insert(pair<string, shared_ptr<Texture>>("Spruce_branches.png", spruceLeaf));

        spruceTrunk = make_shared<Texture>();
  		spruceTrunk->setFilename(resourceDirectory + "/Spruce_obj/Spruce_trunk.jpeg");
  		spruceTrunk->init();
  		spruceTrunk->setUnit(12);
  		spruceTrunk->setWrapModes(GL_REPEAT, GL_REPEAT);
        textureMap.insert(pair<string, shared_ptr<Texture>>("Spruce_trunk.jpeg", spruceTrunk));

        axeTex = make_shared<Texture>();
  		axeTex->setFilename(resourceDirectory + "/axe/axe.png");
  		axeTex->init();
  		axeTex->setUnit(13);
  		axeTex->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

        texture = make_shared<Texture>();
		texture->setFilename(resourceDirectory + "/mask.png");
		texture->init();
		texture->setUnit(14);
		texture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    }

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

        rDir = resourceDirectory;

		// Set background color.
		glClearColor(.72f, .84f, 1.06f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		g_theta = -PI/2.0;

		// Initialize the GLSL program that we will use for local shading
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("MatSpec");
		prog->addUniform("MatShine");
        prog->addUniform("D");
		prog->addUniform("lightPos");
        prog->addUniform("moonLight");
        prog->addUniform("camLight");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");


		// Initialize the GLSL program that we will use for texture mapping
		texProg = make_shared<Program>();
		texProg->setVerbose(true);
		texProg->setShaderNames(resourceDirectory + "/tex_vert.glsl", resourceDirectory + "/tex_frag0.glsl");
		texProg->init();
		texProg->addUniform("P");
		texProg->addUniform("V");
		texProg->addUniform("M");
		texProg->addUniform("flip");
		texProg->addUniform("Texture0");
		texProg->addUniform("MatShine");
        texProg->addUniform("D");
		texProg->addUniform("lightPos");
        texProg->addUniform("moonLight");
        texProg->addUniform("camLight");
		texProg->addAttribute("vertPos");
		texProg->addAttribute("vertNor");
		texProg->addAttribute("vertTex");

        setTextures(resourceDirectory);

        // Initialize the GLSL program that we will use for texture mapping
		cubeProg = make_shared<Program>();
		cubeProg->setVerbose(true);
		cubeProg->setShaderNames(resourceDirectory + "/cube_vert.glsl", resourceDirectory + "/cube_frag.glsl");
		cubeProg->init();
		cubeProg->addUniform("P");
		cubeProg->addUniform("V");
		cubeProg->addUniform("M");
		cubeProg->addUniform("skybox");
		cubeProg->addAttribute("vertPos");
		cubeProg->addAttribute("vertNor");

        progInst = make_shared<Program>();
		progInst->setVerbose(true);
		progInst->setShaderNames(resourceDirectory + "/simple_verti.glsl", resourceDirectory + "/tex_frag0.glsl");
		progInst->init();
		progInst->addUniform("P");
		progInst->addUniform("V");
        progInst->addUniform("Texture0");
		progInst->addUniform("MatShine");
        progInst->addUniform("D");
        progInst->addUniform("lightPos");
        progInst->addUniform("moonLight");
        progInst->addUniform("camLight");
		progInst->addAttribute("vertPos");
		progInst->addAttribute("vertNor");
        progInst->addAttribute("vertTex");
		progInst->addAttribute("instanceMatrix");

        glyphProg = make_shared<Program>();
		glyphProg->setVerbose(true);
        glyphProg->setShaderNames(resourceDirectory + "/glyph_vert.glsl", resourceDirectory + "/glyph_frag.glsl");
		glyphProg->init();
        glyphProg->addUniform("P");
        glyphProg->addUniform("text");
        glyphProg->addUniform("textColor");
        glyphProg->addAttribute("vertex");
        
        writer = new TextGen(&ft, glyphProg);

        // Enable z-buffer test.
		CHECKED_GL_CALL(glEnable(GL_BLEND));
		CHECKED_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		CHECKED_GL_CALL(glPointSize(24.0f));

        partProg = make_shared<Program>();
		partProg->setVerbose(true);
		partProg->setShaderNames(resourceDirectory + "/lab10_vert.glsl", resourceDirectory + "/lab10_frag.glsl");
		partProg->init();
		partProg->addUniform("P");
		partProg->addUniform("M");
		partProg->addUniform("V");
		partProg->addUniform("alphaTexture");
		partProg->addAttribute("vertPos");
        partProg->addAttribute("pColor");

  		// init splines up and down
        splinepath[0] = Spline(glm::vec3(-20,12,-20), glm::vec3(-10,10,-10), glm::vec3(0, 8, 0), glm::vec3(10,6,10), 3);
        splinepath[1] = Spline(glm::vec3(10,6,10), glm::vec3(20,4,20), glm::vec3(25, 2, 30), glm::vec3(16,-1.25+camY,27.25+camZ), 3);

        //engine->play2D("forest.mp3", true); 
	}

    unsigned int createSky(string dir, vector<string> faces) {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(false);
        for(GLuint i = 0; i < faces.size(); i++) {
            unsigned char *data = stbi_load((dir+faces[i]).c_str(), &width, &height, &nrChannels, 0);
            if (data) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            } else {
                cout << "failed to load: " << (dir+faces[i]).c_str() << endl;
            }
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        cout << " creating cube map any errors : " << glGetError() << endl;
        return textureID;
    }

    void initKeyframes() {
        vector<float> arms{-PI/8.0, 0, PI/8.0, 0};
        for (int i=0; i < arms.size(); i++) {
            Keyframe tmp = Keyframe(arms[i], arms[i+1], frameDur, "arm");
            if (i == arms.size()-1)
               tmp = Keyframe(arms[i], arms[0], frameDur, "arm"+std::to_string(i));
            lArmKF.push_back(tmp);
            rArmKF.push_back(tmp);
        }
        vector<float> legs{PI/6.0, 0, -PI/6.0, 0};
        for (int i=0; i < legs.size(); i++) {
            Keyframe tmp = Keyframe(legs[i], legs[i+1], frameDur, "lLeg"+std::to_string(i));
            if (i == legs.size()-1)
                tmp = Keyframe(legs[i], legs[0], frameDur, "lLeg"+std::to_string(i));
            lLegKF.push_back(tmp);

            Keyframe tmp2 = Keyframe(-1*legs[i], -1*legs[i+1], frameDur, "rLeg"+std::to_string(i));
            if (i == legs.size()-1)
                tmp2 = Keyframe(-1*legs[i], -1*legs[0], frameDur, "rLeg"+std::to_string(i));
            rLegKF.push_back(tmp2);
        }
        vector<float> lKnee{0, PI/3.0, 0, 0};
        for (int i=0; i < lKnee.size(); i++) {
            Keyframe tmp = Keyframe(lKnee[i], lKnee[i+1], frameDur, "lKnee"+std::to_string(i));
            if (i == lKnee.size()-1)
                tmp = Keyframe(lKnee[i], lKnee[0], frameDur, "lKnee"+std::to_string(i));
            lKneeKF.push_back(tmp);
        }
        vector<float> rKnee{0, 0, 0, PI/3.0};
        for (int i=0; i < rKnee.size(); i++) {
            Keyframe tmp = Keyframe(rKnee[i], rKnee[i+1], frameDur, "rKnee"+std::to_string(i));
            if (i == rKnee.size()-1)
                tmp = Keyframe(rKnee[i], rKnee[0], frameDur, "rKnee"+std::to_string(i));
            rKneeKF.push_back(tmp);
        }
    }

	void initGeom(const std::string& resourceDirectory)
	{
 		vector<tinyobj::shape_t> TOshapes;
 		vector<tinyobj::material_t> objMaterials;
 		string errStr;
		//load in the mesh and make the shape(s)
 		bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/sphereWTex.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			sphere = make_shared<Shape>();
			sphere->createShape(TOshapes[0]);
			sphere->measure();
			sphere->init();
		}

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/statue2/mm_artdeco_sculpture_01.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			for (int i = 0; i < TOshapes.size(); i++) {
                shared_ptr<Shape> tmp = make_shared<Shape>();
                tmp->createShape(TOshapes[i]);
                tmp->measure();
                tmp->init();

                statueMesh2.push_back(tmp);
            }
		}

        rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/axe/axe.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			for (int i = 0; i < TOshapes.size(); i++) {
                shared_ptr<Shape> tmp = make_shared<Shape>();
                tmp->createShape(TOshapes[i]);
                tmp->measure();
                tmp->init();

                axeMesh.push_back(tmp);
            }
		}

        rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/cube_tex.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			cubeTex = make_shared<Shape>();
			cubeTex->createShape(TOshapes[0]);
			cubeTex->measure();
			cubeTex->init();
		}

        rc = tinyobj::LoadObj(TOshapes, carMat, errStr, (resourceDirectory + "/car/car.obj").c_str(), (resourceDirectory + "/car/").c_str());
        if (!rc) {
			cerr << errStr << endl;
		} else {
            for (int i = 0; i < TOshapes.size(); i++) {
                shared_ptr<Shape> tmp = make_shared<Shape>();
                tmp->createShape(TOshapes[i]);
                tmp->measure();
                tmp->init();
                
                carMesh.push_back(tmp);
            }
		}

        float numSlots = 961;
        modelMatrices = new glm::mat4[numSlots];
        float rowSize = 31.0f;
		for(int i = 0; i < numSlots/rowSize ; i++) {
			for (int j=0; j < (int)rowSize; j++) {
                vec3 tmp = mapSpaces(i, j);
    			glm::mat4 model = glm::mat4(1.0f);
    			mat4 t1 = glm::translate(glm::mat4(1.0f), glm::vec3(tmp.x, tmp.y, tmp.z));
                mat4 r = glm::rotate(glm::mat4(1.0f), (float)(-PI/2), vec3(1, 0, 0));
                mat4 s;
                if (occupancy[i][j] == 1) {
    			    s = glm::scale(glm::mat4(1.0f), glm::vec3(0.45));
                } else {
                    s = glm::scale(glm::mat4(1.0f), glm::vec3(0.0));
                }
    			modelMatrices[i*(int)rowSize+j] = t1*r*s;
    	  	}
		}

        rc = tinyobj::LoadObj(TOshapes, treeMat, errStr, (resourceDirectory + "/Spruce_obj/Spruce.obj").c_str(), (resourceDirectory + "/Spruce_obj/").c_str());
  		if (!rc) {
    		cerr << errStr << endl;
  		} else {
    		for (int i=0; i < TOshapes.size(); i++) {
      		// Initialize each mesh.
      			shared_ptr<ShapeInst> s = make_shared<ShapeInst>(numSlots);
      			s->createShape(TOshapes[i]);
      			s->measure();
      			s->init(modelMatrices);

      			cubeInst.push_back(s);
    		}
  		}

        rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/cube.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			cube = make_shared<Shape>();
			cube->createShape(TOshapes[0]);
			cube->measure();

			cube->init();
		}

        rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/dummy.obj").c_str());
        if (!rc) {
			cerr << errStr << endl;
		} else {
            for (int i = 0; i < TOshapes.size(); i++) {
                shared_ptr<Shape> tmp = make_shared<Shape>();
                tmp->createShape(TOshapes[i]);
                tmp->measure();
                tmp->init();

                findMin(tmp->min.x, tmp->min.y, tmp->min.z);
                findMax(tmp->max.x, tmp->max.y, tmp->max.z);

                dummyMesh.push_back(tmp);
            }
		}

        rc = tinyobj::LoadObj(TOshapes, houseMat, errStr, (resourceDirectory + "/brickHouse/CH_building1.obj").c_str(), (resourceDirectory + "/brickHouse/").c_str());
        if (!rc) {
			cerr << errStr << endl;
		} else {
            for (int i = 0; i < TOshapes.size(); i++) {
                shared_ptr<Shape> tmp = make_shared<Shape>();
                tmp->createShape(TOshapes[i]);
                tmp->measure();
                tmp->init();
                
                houseMesh.push_back(tmp);
            }
		}

        rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/streetlamp.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
            lampMesh = make_shared<Shape>();
            lampMesh->createShape(TOshapes[0]);
            lampMesh->measure();
            lampMesh->init();
		}

        cubeMapTexture = createSky("../resources/cloudy/", faces);

		//code to load in the ground plane (CPU defined data passed to GPU)
		initGround();

        initKeyframes();
	}

    void findMin(float x, float y, float z) {
        if (x < dummyMin.x)
            dummyMin.x = x;
        if (y < dummyMin.y)
            dummyMin.y = y;
        if (z < dummyMin.z)
            dummyMin.z = z;
    }

    void findMax(float x, float y, float z) {
        if (x > dummyMax.x)
            dummyMax.x = x;
        if (y > dummyMax.y)
            dummyMax.y = y;
        if (z > dummyMax.z)
            dummyMax.z = z;
    }

	//directly pass quad for the ground to the GPU
	void initGround() {

		float g_groundSize = 65;
		float g_groundY = -0.25;

  		// A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
		float GrndPos[] = {
			-g_groundSize, g_groundY, -g_groundSize,
			-g_groundSize, g_groundY,  g_groundSize,
			g_groundSize, g_groundY,  g_groundSize,
			g_groundSize, g_groundY, -g_groundSize
		};

		float GrndNorm[] = {
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0
		};

		static GLfloat GrndTex[] = {
      		0, 0, // back
      		0, 1,
      		1, 1,
      		1, 0 };

      	unsigned short idx[] = {0, 1, 2, 0, 2, 3};

		//generate the ground VAO
      	glGenVertexArrays(1, &GroundVertexArrayID);
      	glBindVertexArray(GroundVertexArrayID);

      	g_GiboLen = 6;
      	glGenBuffers(1, &GrndBuffObj);
      	glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
      	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

      	glGenBuffers(1, &GrndNorBuffObj);
      	glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
      	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);

      	glGenBuffers(1, &GrndTexBuffObj);
      	glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
      	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);

      	glGenBuffers(1, &GIndxBuffObj);
     	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
      	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
      }

      //code to draw the ground plane
     void drawGround(shared_ptr<Program> curS) {
     	curS->bind();
     	glBindVertexArray(GroundVertexArrayID);
     	dirt->bind(curS->getUniform("Texture0"));
		//draw the ground plane 
  		SetModel(vec3(0, -1, 0), 0, 0, 1, curS);
  		glEnableVertexAttribArray(0);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
  		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  		glEnableVertexAttribArray(1);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
  		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

  		glEnableVertexAttribArray(2);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
  		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

   		// draw!
  		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
  		glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);

  		glDisableVertexAttribArray(0);
  		glDisableVertexAttribArray(1);
  		glDisableVertexAttribArray(2);
  		curS->unbind();
     }

     //helper function to pass material data to the GPU
	void SetMaterial(shared_ptr<Program> curS, int i) {

    	switch (i) {
    		case 0: //purple
    			glUniform3f(curS->getUniform("MatAmb"), 0.096, 0.046, 0.095);
    			glUniform3f(curS->getUniform("MatDif"), 0.96, 0.46, 0.95);
    			glUniform3f(curS->getUniform("MatSpec"), 0.45, 0.23, 0.45);
    			glUniform1f(curS->getUniform("MatShine"), 120.0);
    		break;
    		case 1: // red rubber
    			glUniform3f(curS->getUniform("MatAmb"), 0.05f, 0.0f, 0.0f);
    			glUniform3f(curS->getUniform("MatDif"), 0.5f, 0.4f, 0.4f);
    			glUniform3f(curS->getUniform("MatSpec"), 0.7f, 0.04f, 0.04f);
    			glUniform1f(curS->getUniform("MatShine"), 10.0);
    		break;
    		case 2: // blue
    			glUniform3f(curS->getUniform("MatAmb"), 0.004, 0.05, 0.09);
    			glUniform3f(curS->getUniform("MatDif"), 0.04, 0.5, 0.9);
    			glUniform3f(curS->getUniform("MatSpec"), 0.02, 0.25, 0.45);
    			glUniform1f(curS->getUniform("MatShine"), 27.9);
    		break;
            case 3: // perl
                glUniform3f(curS->getUniform("MatAmb"), 0.25f, 0.20725f, 0.20725f);
    			glUniform3f(curS->getUniform("MatDif"), 1.0f, 0.829f, 0.829f);
    			glUniform3f(curS->getUniform("MatSpec"), 0.296648f, 0.296648f, 0.296648f);
    			glUniform1f(curS->getUniform("MatShine"), 11.264f);
            break;
            case 4: // black rubber
                glUniform3f(curS->getUniform("MatAmb"), 0.02f, 0.02f, 0.02f);
    			glUniform3f(curS->getUniform("MatDif"), 0.01f, 0.01f, 0.01f);
    			glUniform3f(curS->getUniform("MatSpec"), 0.4f, 0.4f, 0.4f);
    			glUniform1f(curS->getUniform("MatShine"), 10.0f);
            break;
  		}
	}

    void SetGenericMat(shared_ptr<Program> curS, float ambient[3], float diffuse[3], float specular[3], float shininess, string type) {
        if (type == "house" || type == "tree") {
            glUniform1f(curS->getUniform("MatShine"), shininess);
        } else {
            if (type == "car") {
                glUniform3f(curS->getUniform("MatAmb"), 0.05, 0.05, 0.05);
            } else {
                glUniform3f(curS->getUniform("MatAmb"), ambient[0], ambient[1], ambient[2]);
            }
            glUniform3f(curS->getUniform("MatDif"), diffuse[0], diffuse[1], diffuse[2]);
            glUniform3f(curS->getUniform("MatSpec"), specular[0], specular[1], specular[2]);
            glUniform1f(curS->getUniform("MatShine"), shininess);
        }
    }

	/* helper function to set model trasnforms */
  	void SetModel(vec3 trans, float rotY, float rotX, float sc, shared_ptr<Program> curS) {
  		mat4 Trans = glm::translate( glm::mat4(1.0f), trans);
  		mat4 RotX = glm::rotate( glm::mat4(1.0f), rotX, vec3(1, 0, 0));
  		mat4 RotY = glm::rotate( glm::mat4(1.0f), rotY, vec3(0, 1, 0));
  		mat4 ScaleS = glm::scale(glm::mat4(1.0f), vec3(sc));
  		mat4 ctm = Trans*RotX*RotY*ScaleS;
  		glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
  	}

	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   	}

   	/* camera controls - do not change */
	void SetView(shared_ptr<Program>  shader) {
  		glm::mat4 Cam = glm::lookAt(g_eye, g_lookAt, vec3(0, 1, 0));
  		glUniformMatrix4fv(shader->getUniform("V"), 1, GL_FALSE, value_ptr(Cam));
	}

    void drawHouse(shared_ptr<MatrixStack> Model, shared_ptr<Program> drawProg) {
        Model->pushMatrix();
            Model->pushMatrix();
                vec3 pos = mapSpaces(0, 17);
                //Model->translate(vec3(0, -1.25, -7));
                Model->translate(vec3(pos.x, -1.25, pos.z - 10));
                Model->scale(vec3(0.35, 0.35, 0.35));

                setModel(drawProg, Model);
                for (int i=0; i < houseMesh.size(); i++) {
                    int mat = houseMesh[i]->getMat()[0];
                    SetGenericMat(drawProg, houseMat[mat].ambient, houseMat[mat].diffuse, houseMat[mat].specular, houseMat[mat].shininess, "house");
                    if (houseMat[mat].diffuse_texname != "") {
                        if (i == 260 || i == 264) {
                            glUniform1i(drawProg->getUniform("flip"), 0);
                        } else {
                            glUniform1i(drawProg->getUniform("flip"), 1);
                        } 
                        if (houseMat[mat].diffuse_texname == "063.JPG") {
                            lightWood->bind(drawProg->getUniform("Texture0"));
                        } else {
                            textureMap.at(houseMat[mat].diffuse_texname)->bind(drawProg->getUniform("Texture0"));
                        }
                    } else {
                        if (mat == 10) {
                            glass->bind(drawProg->getUniform("Texture0"));
                        } else if (mat == 4) {
                            blackText->bind(drawProg->getUniform("Texture0"));
                        } else {
                            whiteWood->bind(drawProg->getUniform("Texture0"));
                        }
                    }
                    houseMesh[i]->draw(drawProg);
                }
            Model->popMatrix();
        Model->popMatrix();
    }

    vec3 mapSpaces(int i, int j) {
        float x = (2*j) - 30;
        float z = (2*i) - 30;
        return vec3(x, -1.2, z);
    }

    vec3 findCenter(int i) {
        float x = (dummyMesh[i]->max.x + dummyMesh[i]->min.x)/2;
        float y = (dummyMesh[i]->max.y + dummyMesh[i]->min.y)/2;
        float z = (dummyMesh[i]->max.z + dummyMesh[i]->min.z)/2;
        return vec3(x, y, z);
    }

    void interpolateGroup() {
        if (isNewFrame) {
            isNewFrame = false;
            startTime = glfwGetTime();
        }
        float currTime = glfwGetTime();
        float percentDone = (currTime-startTime) / frameDur;
        if (percentDone > 1) {
            overallFrame = (++overallFrame)%4;
            isNewFrame = true;
            if (overallFrame == 1 || overallFrame == 3)
                isWalking = false; 
        } else {
            for (int i=0; i < limbRot.size(); i++) {
                Keyframe k;
                if (i == 0)
                    k = lArmKF[overallFrame];
                else if (i == 1)
                    k = rArmKF[overallFrame];
                else if (i == 2)
                    k = lLegKF[overallFrame];
                else if (i == 3)
                    k = lKneeKF[overallFrame];
                else if (i == 4)
                    k = rLegKF[overallFrame];
                else
                    k = rKneeKF[overallFrame];
                float diff = k.returnEnd() - k.returnStart();
                limbRot[i] = k.returnStart() + (percentDone*diff); 
            }
        }
    }

    void drawLeftArm(shared_ptr<MatrixStack> Model, shared_ptr<Program> prog, float frametime) {
        //  KEYFRAMES X-AXIS: -PI/2.4 (arm to side),
        //  KEYFRAMES Z-AXIS: none, PI/8.0, none, -PI/8.0
        Model->pushMatrix();
            Model->translate(vec3(1.0f*dummyMesh[21]->min.x, 1.0f*dummyMesh[21]->min.y, 1.0f*dummyMesh[21]->max.z));
            Model->rotate(-PI/2.4, vec3(1, 0, 0));
            Model->rotate(limbRot[0], vec3(0, 0, 1));
            Model->translate(vec3(-1.0f*dummyMesh[21]->min.x, -1.0f*dummyMesh[21]->min.y, -1.0f*dummyMesh[21]->max.z));
            setModel(prog, Model);
            for (int i=21; i <=26; i++) {
                if (i < 23)
                    SetMaterial(prog, 1);
                else
                    SetMaterial(prog, 3);
                dummyMesh[i]->draw(prog);
            }
        Model->popMatrix();
    }

    void drawRightArm(shared_ptr<MatrixStack> Model, shared_ptr<Program> prog, float frametime) {
        //  KEYFRAMES X-AXIS: PI/2.4 (arm to side)
        //  KEYFRAMES Z-AXIS: none, PI/8.0, none, -PI/8.0
        Model->pushMatrix();
            Model->translate(vec3(1.0f*dummyMesh[15]->max.x, 1.0f*dummyMesh[15]->max.y, 1.0f*dummyMesh[15]->max.z));
            Model->rotate(PI/2.4, vec3(1, 0, 0));
            Model->rotate(limbRot[1], vec3(0, 0, 1));
            Model->translate(vec3(-1.0f*dummyMesh[15]->max.x, -1.0f*dummyMesh[15]->max.y, -1.0f*dummyMesh[15]->max.z));
            setModel(prog, Model);
            for (int i=15; i <=20; i++) {
                if (i < 17)
                    SetMaterial(prog, 1);
                else
                    SetMaterial(prog, 3);
                dummyMesh[i]->draw(prog);
            }
        Model->popMatrix();
    }

    void drawLeftLeg(shared_ptr<MatrixStack> Model, shared_ptr<Program> prog, float frametime) {
        //  KEYFRAMES: PI/6.0 (leg backward), none (but knee bent), -PI/10.0, -PI/6.0, none,
        Model->pushMatrix();
            Model->translate(vec3(1.0f*dummyMesh[11]->min.x, 1.0f*dummyMesh[11]->min.y, 1.0f*dummyMesh[11]->max.z));
            Model->rotate(limbRot[2], vec3(0, 1, 0));
            Model->translate(vec3(-1.0f*dummyMesh[11]->min.x, -1.0f*dummyMesh[11]->min.y, -1.0f*dummyMesh[11]->max.z));
            setModel(prog, Model);
            SetMaterial(prog, 2);
            dummyMesh[11]->draw(prog); // pelvis
            dummyMesh[10]->draw(prog); // upper leg

            // KEYFRAMES: none, PI/3.0, PI/4.0, none, none, 
            Model->translate(vec3(1.0f*dummyMesh[9]->max.x, 1.0f*dummyMesh[9]->min.y, 1.0f*dummyMesh[9]->max.z));
            Model->rotate(limbRot[3], vec3(0, 1, 0));
            Model->translate(vec3(-1.0f*dummyMesh[9]->max.x, -1.0f*dummyMesh[9]->min.y, -1.0f*dummyMesh[9]->max.z));
            setModel(prog, Model);
            SetMaterial(prog, 2);
            for (int i=6; i <=9; i++) {
                if (i == 6)
                    SetMaterial(prog, 4);
                else
                    SetMaterial(prog, 2);
                dummyMesh[i]->draw(prog);
            }
        Model->popMatrix();
    }

    void drawRightLeg(shared_ptr<MatrixStack> Model, shared_ptr<Program> prog, float frametime) {
        //  KEYFRAMES: -PI/6.0 (leg forward), none, none, PI/6.0, none (but knee bent)
        Model->pushMatrix();
            Model->translate(vec3(1.0f*dummyMesh[5]->max.x, 1.0f*dummyMesh[5]->max.y, 1.0f*dummyMesh[5]->max.z));
            Model->rotate(limbRot[4], vec3(0, 1, 0));
            Model->translate(vec3(-1.0f*dummyMesh[5]->max.x, -1.0f*dummyMesh[5]->max.y, -1.0f*dummyMesh[5]->max.z));
            setModel(prog, Model);
            SetMaterial(prog, 2);
            dummyMesh[5]->draw(prog); // pelvis
            dummyMesh[4]->draw(prog); // upper leg

            // KEYFRAMES: none, none, none, none, PI/3.0
            Model->translate(vec3(1.0f*dummyMesh[9]->max.x, 1.0f*dummyMesh[9]->min.y, 1.0f*dummyMesh[9]->max.z));
            Model->rotate(limbRot[5], vec3(0, 1, 0));
            Model->translate(vec3(-1.0f*dummyMesh[9]->max.x, -1.0f*dummyMesh[9]->min.y, -1.0f*dummyMesh[9]->max.z));
            setModel(prog, Model);
            for (int i=0; i <=3; i++) {
                if (i == 0)
                    SetMaterial(prog, 4);
                else
                    SetMaterial(prog, 2);
                dummyMesh[i]->draw(prog);
            }
        Model->popMatrix();
    }

    void drawDummy(shared_ptr<MatrixStack> Model, shared_ptr<Program> prog, float frametime) {
        if (isWalking)
            interpolateGroup();
        Model->pushMatrix();
            Model->translate(vec3(dummyLoc.x, dummyLoc.y, dummyLoc.z));
            Model->scale(vec3(0.01, 0.01, 0.01));
            Model->rotate(dummyRot, vec3(0, 1, 0));
            // Model->rotate(-PI/2.0, vec3(0, 1, 0)); FIRST PERSON
            Model->rotate(-PI/2.0, vec3(1, 0, 0));
            setModel(prog, Model);
            for (int i=12; i <= 14; i++) {
                if (i == 12)
                    SetMaterial(prog, 2);
                else
                    SetMaterial(prog, 1);
                dummyMesh[i]->draw(prog);
            }
            SetMaterial(prog, 3);
            dummyMesh[27]->draw(prog); // neck
            dummyMesh[28]->draw(prog); // head

            drawLeftArm(Model, prog, frametime);
            drawRightArm(Model, prog, frametime);
            
            drawLeftLeg(Model, prog, frametime);
            drawRightLeg(Model, prog, frametime);
        Model->popMatrix();
    }

    void drawCar(shared_ptr<MatrixStack> Model, shared_ptr<Program> prog) {
        driveTheta = 1.5*sin(glfwGetTime());

        Model->pushMatrix();
            vec3 pos = mapSpaces(0, 17);
            Model->translate(vec3(pos.x, -0.5, pos.z - 4));
            //Model->translate(vec3(2.7, -0.85, 2));
            Model->translate(vec3(driveTheta, 0, 0));
            Model->rotate(PI/2.0, vec3(0, 1, 0));
            Model->scale(vec3(0.55, 0.55, 0.55));

            setModel(prog, Model);
            float diffuse[3] = {0.840000, 0.332781, 0.311726};
            for (int i=0; i < carMesh.size(); i++) {
                int mat = carMesh[i]->getMat()[0];
                SetGenericMat(prog, carMat[mat].ambient, carMat[mat].diffuse, carMat[mat].specular, carMat[mat].shininess, "car");
                carMesh[i]->draw(prog);
            }
        Model->popMatrix();
    }

   	void updateUsingCameraPath(float frametime)  {

   	  if (goCamera) {
        g_lookAt = vec3(dummyLoc.x, dummyLoc.y+camY, dummyLoc.z);
        if(!splinepath[0].isDone()){
       		splinepath[0].update(frametime);
            g_eye = splinepath[0].getPosition();
        } else {
            splinepath[1].update(frametime);
            g_eye = splinepath[1].getPosition();
            if (splinepath[1].isDone()) {
                goCamera = false;
                gameStart = glfwGetTime();
            }
        }
      }
   	}

    void readMaze(const std::string& resourceDirectory) {
        string slot;
        int i = 0;
        ifstream myfile(resourceDirectory + "/map.txt");

        if (myfile.is_open()) {
            while (getline(myfile, slot)) {
                for (int j=0; j < slot.length(); j++) {
                    if (slot[j] == 'x') {
                        occupancy[i][j] = 1;
                    } else {
                        occupancy[i][j] = 0;
                    }
                }
                i += 1;
            }
            myfile.close();
        } else {
            cout << "Unable to open file" << endl;
        }
    }

    string formatTime() {
        float totalTime = 0;
        if (timedGame)
            totalTime = timer;
        else
            totalTime = gameEnd - gameStart;
        int min = totalTime/60;
        int sec = (int)totalTime%60;
        if (sec < 10)
            return std::to_string(min)+":0"+std::to_string(sec);
        else
            return std::to_string(min)+":"+std::to_string(sec);
    }

    void updateTimer(float frametime) {
        timer -= frametime;
        if (timer <= 0) {
            lostGame = true;
        }
    }

	void render(float frametime) {
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Use the matrix stack for Lab 6
		float aspect = width/(float)height;

		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

		//update the camera position
        if (!startScreen)
		    updateUsingCameraPath(frametime);

		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 100.0f);

        if (timedGame && !lostGame && !gameOver) {
            updateTimer(frametime);
        }

		texProg->bind();
            glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
            SetView(texProg);
            glUniform3f(texProg->getUniform("moonLight"), 0, 8, 0);
            glUniform3f(texProg->getUniform("camLight"), g_eye.x, g_eye.y, g_eye.z);
            glUniform3f(texProg->getUniform("lightPos"), dummyLoc.x, dummyLoc.y+camY, dummyLoc.z);
            glUniform3f(texProg->getUniform("D"), dummyLoc.x + gaze.x, dummyLoc.y + gaze.y, dummyLoc.z + gaze.z);
            glUniform1i(texProg->getUniform("flip"), 1);

            // Model->pushMatrix();
            //     Model->translate(vec3(14.5, -1.25, 31.25));
            //     Model->rotate(-PI/2, vec3(0, 1, 0));
            //     Model->scale(vec3(0.0125, 0.0125, 0.0125));
            //     statueTex2->bind(texProg->getUniform("Texture0"));
            //     glUniformMatrix4fv(texProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
            //     for (int i=0; i < statueMesh.size(); i++) {
            //         statueMesh2[i]->draw(texProg);
            //     }
            // Model->popMatrix();

            // Model->pushMatrix();
            //     Model->translate(vec3(17.5, -1.25, 31.25));
            //     Model->rotate(-PI/2, vec3(0, 1, 0));
            //     Model->scale(vec3(0.0125, 0.0125, 0.0125));
            //     statueTex2->bind(texProg->getUniform("Texture0"));
            //     glUniformMatrix4fv(texProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
            //     for (int i=0; i < statueMesh.size(); i++) {
            //         statueMesh2[i]->draw(texProg);
            //     }
            // Model->popMatrix();

            //drawHouse(Model, texProg);

            // hedge->bind(texProg->getUniform("Texture0"));
            // for (int i=0; i < 31; i++) {
            //     for (int j=0; j < 31; j++) {
            //         if (occupancy[i][j] == 1) {
            //             vec3 tmp = mapSpaces(i, j);
            //             Model->pushMatrix();
            //             Model->translate(vec3(tmp.x, 0, tmp.z));
            //             Model->scale(vec3(2, 2, 2));
            //             glUniformMatrix4fv(texProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
            //             cubeTex->draw(texProg);
            //             Model->popMatrix();
            //         }
            //     }
            // }

            for (unsigned int j=0; j < 3; j++) {
                Model->pushMatrix();
                    Model->translate(vec3(axe_x[j], 0, axe_z[j]));
                    if (j == 2)
                        Model->rotate(PI/2.0, vec3(0, 1, 0));
                    Model->rotate(-PI/4.0, vec3(0, 0, 1));
                    Model->scale(vec3(axe_scale[j]));
                    axeTex->bind(texProg->getUniform("Texture0"));
                    setModel(texProg, Model);
                    for (int i=0; i < axeMesh.size(); i++) {   
                        axeMesh[i]->draw(texProg);
                    }
                Model->popMatrix();
            }

            glUniform1i(texProg->getUniform("flip"), 1);
            drawGround(texProg);

		texProg->unbind();

        progInst->bind();
            glUniformMatrix4fv(progInst->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
            SetView(progInst);
            glUniform3f(progInst->getUniform("moonLight"), 0, 8, 0);
            glUniform3f(progInst->getUniform("camLight"), g_eye.x, g_eye.y, g_eye.z);
            glUniform3f(progInst->getUniform("lightPos"), dummyLoc.x, dummyLoc.y+camY, dummyLoc.z);
            glUniform3f(progInst->getUniform("D"), dummyLoc.x + gaze.x, dummyLoc.y + gaze.y, dummyLoc.z + gaze.z);
            for (int i=0; i < cubeInst.size(); i++) {   
                int mat = cubeInst[i]->getMat()[0];
                SetGenericMat(progInst, treeMat[mat].ambient, treeMat[mat].diffuse, treeMat[mat].specular, treeMat[mat].shininess, "tree");
                if (treeMat[mat].diffuse_texname != "") {
                    textureMap.at(treeMat[mat].diffuse_texname)->bind(progInst->getUniform("Texture0"));
                }
                cubeInst[i]->draw(progInst);
            }
		progInst->unbind();

        //to draw the sky box bind the right shader
        cubeProg->bind();
            glUniformMatrix4fv(cubeProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
            glDepthFunc(GL_LEQUAL);
            SetView(cubeProg);
            Model->pushMatrix();
            Model->translate(vec3(0, -3, 0));
            Model->rotate(3.1416, vec3(0, 1, 0));
            Model->scale(vec3(80, 80, 80));
            glUniformMatrix4fv(cubeProg->getUniform("M"), 1, GL_FALSE,value_ptr(Model->topMatrix()));
            Model->popMatrix();
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);

            cube->draw(cubeProg);

            glDepthFunc(GL_LESS);
        cubeProg->unbind(); 

        prog->bind();
            glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
            glUniform3f(prog->getUniform("moonLight"), 0, 8, 0);
            glUniform3f(prog->getUniform("camLight"), g_eye.x, g_eye.y, g_eye.z);
            glUniform3f(prog->getUniform("lightPos"), dummyLoc.x, dummyLoc.y+camY, dummyLoc.z);
            glUniform3f(prog->getUniform("D"), dummyLoc.x + gaze.x, dummyLoc.y + gaze.y, dummyLoc.z + gaze.z);
            SetView(prog);
            drawDummy(Model, prog, frametime);
            drawCar(Model, prog);
        prog->unbind();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

        glyphProg->bind();
            glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(width), 0.0f, static_cast<GLfloat>(height));
            glUniformMatrix4fv(glyphProg->getUniform("P"), 1, GL_FALSE, value_ptr(projection));
            if (startScreen) {
                writer->drawText(1, "Escape the Forest", width/2, height/2+85.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
                writer->drawText(1, "Select Mode (Press 1 or 2):", width/2, height/2-20.0f, 0.45f, glm::vec3(1.0f, 1.0f, 1.0f));
                writer->drawText(1, "Explore Mode (1): See if you can escape the maze!", width/2, height/2-45.0f, 0.35f, glm::vec3(1.0f, 1.0f, 1.0f));
                writer->drawText(1, "Timed Mode (2): See if you can escape the maze before time runs out!", width/2, height/2-65.0f, 0.35f, glm::vec3(1.0f, 1.0f, 1.0f));
            } else {
                if (gameOver) {
                    writer->drawText(1, "You Won!", width/2, height/2+85.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
                    if (timedGame){
                        writer->drawText(1, "Time Left: "+formatTime(), width/2, height/2+60.0f, 0.35f, glm::vec3(1.0f, 1.0f, 1.0f));
                    } else {
                        writer->drawText(1, "Time Taken: "+formatTime(), width/2, height/2+60.0f, 0.35f, glm::vec3(1.0f, 1.0f, 1.0f));
                    }
                    writer->drawText(1, "Press R to restart", width/2, height/2+10.0f, 0.45f, glm::vec3(1.0f, 1.0f, 1.0f));
                } else {
                    writer->drawText(1, "Axes: "+std::to_string(hasAxe), 50.0f, 10.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
                }
                if (timedGame) {
                    if (lostGame) {
                        writer->drawText(1, "You Lost :(", width/2, height/2+85.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
                        writer->drawText(1, "You didn't finish the maze before the time was up.", width/2, height/2+60.0f, 0.35f, glm::vec3(1.0f, 1.0f, 1.0f));
                        writer->drawText(1, "Press R to restart", width/2, height/2+10.0f, 0.45f, glm::vec3(1.0f, 1.0f, 1.0f));
                    } else {
                        writer->drawText(1, formatTime(), 600.0f, 10.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
                    }
                }
            }
        glyphProg->unbind();

        if(isChopping) {
            partProg->bind();
                texture->bind(partProg->getUniform("alphaTexture"));
                CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix())));
                SetView(partProg);
                glUniformMatrix4fv(partProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
                
                thePartSystem->drawMe(partProg);
                thePartSystem->update();

            partProg->unbind();
        }

        glDisable(GL_BLEND);

		// Pop matrix stacks.
		Projection->popMatrix();

	}
};

int main(int argc, char *argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

    //ISoundEngine* engine = createIrrKlangDevice();

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

    application->readMaze(resourceDir);

	application->init(resourceDir);
	application->initGeom(resourceDir);

	auto lastTime = chrono::high_resolution_clock::now();
	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// save current time for next frame
		auto nextLastTime = chrono::high_resolution_clock::now();

		// get time since last frame
		float deltaTime =
			chrono::duration_cast<std::chrono::microseconds>(
				chrono::high_resolution_clock::now() - lastTime)
				.count();
		// convert microseconds (weird) to seconds (less weird)
		deltaTime *= 0.000001;

		// reset lastTime so that we can calculate the deltaTime
		// on the next frame
		lastTime = nextLastTime;

		// Render scene.
		application->render(deltaTime);
		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}
    //engine->drop();

	// Quit program.
	windowManager->shutdown();
	return 0;
}
