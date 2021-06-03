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

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>
#define PI 3.1415927

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

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
    vector<shared_ptr<Shape>> statueMesh;
    vector<shared_ptr<Shape>> statueMesh2;
    vector<shared_ptr<Shape>> axeMesh;
    vector<vec3> dummyMin;
    vector<vec3> dummyMax;

    vector<tinyobj::material_t> houseMat;
    vector<tinyobj::material_t> carMat;
    vector<tinyobj::material_t> treeMat;
    map<string, shared_ptr<Texture>> textureMap;

    float driveTheta = 0;
    int frame = 0;
    bool newFrame = true;
    float frameDur = 2;
    float startTime;
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
	shared_ptr<Texture> texture0;
	shared_ptr<Texture> brownWood;
    shared_ptr<Texture> brick;
    shared_ptr<Texture> blackText;
    shared_ptr<Texture> lightWood;
    shared_ptr<Texture> leaf;
    shared_ptr<Texture> road;
    shared_ptr<Texture> bumpBrick;
    shared_ptr<Texture> whiteText;
    shared_ptr<Texture> tiles;
    shared_ptr<Texture> stone;
    shared_ptr<Texture> lamp;
    shared_ptr<Texture> whiteWood;
    shared_ptr<Texture> glass;
    shared_ptr<Texture> bark;
    shared_ptr<Texture> bigLeaf;
    shared_ptr<Texture> hedge;
    shared_ptr<Texture> dirt;
    shared_ptr<Texture> statueTex;
    shared_ptr<Texture> statueTex2;
    shared_ptr<Texture> spruceLeaf;
    shared_ptr<Texture> spruceTrunk;
    shared_ptr<Texture> firLeaf;
    shared_ptr<Texture> firTrunk;
    shared_ptr<Texture> axeTex;

	//animation data
	float lightTrans = 0;

    int occupancy[31][31] = {0};
    vector<float> positions;
    glm::mat4 *modelMatrices;
    int numWalls = 0;

    bool setLaunch = true;

	//camera
	double g_phi, g_theta;
	vec3 view = vec3(0, 0, 1);
	vec3 strafe = vec3(1, 0, 0);
	vec3 g_eye = vec3(16, 0, 33);
    // vec3 g_eye = vec3(mapSpaces(11, 15).x, 0, mapSpaces(11, 15).z);
    // vec3 g_eye = vec3(0, 60, 10);
    // vec3 g_lookAt = vec3(0, 0, 0);
	vec3 g_lookAt = vec3(16, 0, 30);
    float speed = 0.3;

    vec3 dummyLoc = vec3(16, -1.25, 30);

    // Chopping actions
    int hasAxe = 0;
    bool firstAct = true;
    float axe_scale[3] = {0.001, 0.001, 0.001};
    float axe_x[3] = {mapSpaces(11, 15).x, mapSpaces(13, 5).x, mapSpaces(27, 17).x};
    float axe_z[3] = {mapSpaces(11, 15).z, mapSpaces(13, 5).z, mapSpaces(27, 17).z};
    bool axe_taken[3] = {false, false, false};

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
        if (key == GLFW_KEY_T && action == GLFW_PRESS) { // Chopping down trees
            vec2 tmp = findMySpace(g_eye);
            if (hasAxe > 0 && firstAct) {
                vec2 chopLoc = findMySpace(g_lookAt);
                float i = chopLoc.x;
                float j = chopLoc.y;
                vec3 view = g_eye-g_lookAt;
                cout << "VIEW: " << view.x << endl;
                cout << "VIEW: " << view.z << endl;
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
                cout << i << endl;
                cout << j << endl;
                if (occupancy[(int)i][(int)j] != 0) { // Only act if facing a tree
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
			goCamera = !goCamera;
            if (!goCamera)
                computeLookAt();
		}
        if (key == GLFW_KEY_W && action == GLFW_PRESS){
			view = g_lookAt - g_eye;
            if (!detectCollision(g_eye + (speed*view)) && !detectHeight(g_eye + (speed*view))) {
                // dummyLoc = dummyLoc + (speed*view);
                g_eye = g_eye + (speed*view);
                g_lookAt = g_lookAt + (speed*view);
            }
		}
        if (key == GLFW_KEY_A && action == GLFW_PRESS){
            view = g_lookAt - g_eye;
            strafe = cross(view, vec3(0, 1, 0));
            if (!detectCollision(g_eye - (speed*strafe)) && !detectHeight(g_eye - (speed*strafe))) {
                // dummyLoc = dummyLoc - (speed*strafe);
                g_eye = g_eye - (speed*strafe);
                g_lookAt = g_lookAt - (speed*strafe);
            }
		}
        if (key == GLFW_KEY_S && action == GLFW_PRESS){
			view = g_lookAt - g_eye;
            if (!detectCollision(g_eye - (speed*view)) && !detectHeight(g_eye - (speed*view))) {
                // dummyLoc = dummyLoc - (speed*view);
                g_eye = g_eye - (speed*view);
                g_lookAt = g_lookAt - (speed*view);
            }
		}
        if (key == GLFW_KEY_D && action == GLFW_PRESS){
            view = g_lookAt - g_eye;
            strafe = cross(view, vec3(0, 1, 0));
            if (!detectCollision(g_eye + (speed*strafe)) && !detectHeight(g_eye + (speed*strafe))) {
                // dummyLoc = dummyLoc + (speed*strafe);
                g_eye = g_eye + (speed*strafe);
                g_lookAt = g_lookAt + (speed*strafe);
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
        if (occupancy[(int)occPos.x][(int)occPos.y] != 1) // If not a wall
            return false;
        else
            return true;
    }

    bool detectHeight(vec3 myPos) {
        if (myPos.y > 0.5 || myPos.y < -0.05)
            return true;
        else
            return false;
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
        if (!goCamera) {
            g_theta -= deltaX/100;
            if (g_phi < DegToRad(80) && g_phi > -DegToRad(80)) {
                g_phi += deltaY/100;
            }
            if (g_phi >= DegToRad(80) && deltaY < 0) {
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
        float radius = 1.0;
        float x = radius*cos(g_phi)*cos(g_theta);
        float y = radius*sin(g_phi);
        float z = radius*cos(g_phi)*cos((PI/2.0)-g_theta);
        g_lookAt = vec3(x, y, z) + g_eye;
    }

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

    void setTextures(const std::string& resourceDirectory) {
        //read in a load the texture
		texture0 = make_shared<Texture>();
  		texture0->setFilename(resourceDirectory + "/grass_low.jpg");
  		texture0->init();
  		texture0->setUnit(0);
  		texture0->setWrapModes(GL_REPEAT, GL_REPEAT);

  		brownWood = make_shared<Texture>();
  		brownWood->setFilename(resourceDirectory + "/cartoonWood.jpg");
  		brownWood->init();
  		brownWood->setUnit(1);
  		brownWood->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

        brick = make_shared<Texture>();
  		brick->setFilename(resourceDirectory + "/brickHouse/campiangatebrick1.jpg");
  		brick->init();
  		brick->setUnit(2);
  		brick->setWrapModes(GL_REPEAT, GL_REPEAT);
        textureMap.insert(pair<string, shared_ptr<Texture>>("campiangatebrick1.jpg", brick));

        blackText = make_shared<Texture>();
  		blackText->setFilename(resourceDirectory + "/brickHouse/063.jpg");
  		blackText->init();
  		blackText->setUnit(3);
  		blackText->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        textureMap.insert(pair<string, shared_ptr<Texture>>("063.JPG", blackText));

        lightWood = make_shared<Texture>();
  		lightWood->setFilename(resourceDirectory + "/brown_wood.jpg");
  		lightWood->init();
  		lightWood->setUnit(4);
  		lightWood->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

        leaf = make_shared<Texture>();
  		leaf->setFilename(resourceDirectory + "/tree/maple_leaf.png");
  		leaf->init();
  		leaf->setUnit(5);
  		leaf->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

        road = make_shared<Texture>();
  		road->setFilename(resourceDirectory + "/driveway2.jpg");
  		road->init();
  		road->setUnit(6);
  		road->setWrapModes(GL_REPEAT, GL_REPEAT);

        bumpBrick = make_shared<Texture>();
  		bumpBrick->setFilename(resourceDirectory + "/brickHouse/campiangatebrick1_bump.jpg");
  		bumpBrick->init();
  		bumpBrick->setUnit(7);
  		bumpBrick->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        textureMap.insert(pair<string, shared_ptr<Texture>>("campiangatebrick1_bump.jpg", bumpBrick));

        whiteText = make_shared<Texture>();
  		whiteText->setFilename(resourceDirectory + "/brickHouse/HighBuild_texture.jpg");
  		whiteText->init();
  		whiteText->setUnit(8);
  		whiteText->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        textureMap.insert(pair<string, shared_ptr<Texture>>("HighBuild_texture.jpg", whiteText));

        tiles = make_shared<Texture>();
  		tiles->setFilename(resourceDirectory + "/brickHouse/panTiles_1024_more_red.jpg");
  		tiles->init();
  		tiles->setUnit(9);
  		tiles->setWrapModes(GL_REPEAT, GL_REPEAT);
        textureMap.insert(pair<string, shared_ptr<Texture>>("panTiles_1024_more_red.jpg", tiles));

        stone = make_shared<Texture>();
  		stone->setFilename(resourceDirectory + "/brickHouse/stones006x04.jpg");
  		stone->init();
  		stone->setUnit(10);
  		stone->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        textureMap.insert(pair<string, shared_ptr<Texture>>("stones006x04.jpg", stone));

        lamp = make_shared<Texture>();
  		lamp->setFilename(resourceDirectory + "/diffuse_streetlamp.jpg");
  		lamp->init();
  		lamp->setUnit(11);
  		lamp->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

        whiteWood = make_shared<Texture>();
  		whiteWood->setFilename(resourceDirectory + "/white_wood.jpg");
  		whiteWood->init();
  		whiteWood->setUnit(12);
  		whiteWood->setWrapModes(GL_REPEAT, GL_REPEAT);

        glass = make_shared<Texture>();
  		glass->setFilename(resourceDirectory + "/glass.jpg");
  		glass->init();
  		glass->setUnit(13);
  		glass->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

        hedge = make_shared<Texture>();
  		hedge->setFilename(resourceDirectory + "/hedge.jpg");
  		hedge->init();
  		hedge->setUnit(14);
  		hedge->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

        dirt = make_shared<Texture>();
  		dirt->setFilename(resourceDirectory + "/ground.jpg");
  		dirt->init();
  		dirt->setUnit(15);
  		dirt->setWrapModes(GL_REPEAT, GL_REPEAT);

        statueTex = make_shared<Texture>();
  		statueTex->setFilename(resourceDirectory + "/statue/statue.jpg");
  		statueTex->init();
  		statueTex->setUnit(16);
  		statueTex->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

        statueTex2 = make_shared<Texture>();
  		statueTex2->setFilename(resourceDirectory + "/statue2/mm_facade_sculpture_03_diffus.jpg");
  		statueTex2->init();
  		statueTex2->setUnit(17);
  		statueTex2->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

        spruceLeaf = make_shared<Texture>();
  		spruceLeaf->setFilename(resourceDirectory + "/Spruce_obj/Spruce_branches.png");
  		spruceLeaf->init();
  		spruceLeaf->setUnit(18);
  		spruceLeaf->setWrapModes(GL_REPEAT, GL_REPEAT);
        textureMap.insert(pair<string, shared_ptr<Texture>>("Spruce_branches.png", spruceLeaf));

        spruceTrunk = make_shared<Texture>();
  		spruceTrunk->setFilename(resourceDirectory + "/Spruce_obj/Spruce_trunk.jpeg");
  		spruceTrunk->init();
  		spruceTrunk->setUnit(19);
  		spruceTrunk->setWrapModes(GL_REPEAT, GL_REPEAT);
        textureMap.insert(pair<string, shared_ptr<Texture>>("Spruce_trunk.jpeg", spruceTrunk));

        firLeaf = make_shared<Texture>();
  		firLeaf->setFilename(resourceDirectory + "/fir/branch.png");
  		firLeaf->init();
  		firLeaf->setUnit(20);
  		firLeaf->setWrapModes(GL_REPEAT, GL_REPEAT);

        firTrunk = make_shared<Texture>();
  		firTrunk->setFilename(resourceDirectory + "/fir/bark.jpg");
  		firTrunk->init();
  		firTrunk->setUnit(21);
  		firTrunk->setWrapModes(GL_REPEAT, GL_REPEAT);

        axeTex = make_shared<Texture>();
  		axeTex->setFilename(resourceDirectory + "/axe/axe.png");
  		axeTex->init();
  		axeTex->setUnit(22);
  		axeTex->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    }

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

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
		prog->addUniform("lightPos");
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
		texProg->addUniform("lightPos");
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
        // progInst->addUniform("flip");
        // progInst->addUniform("MatAmb");
		// progInst->addUniform("MatDif");
		// progInst->addUniform("MatSpec");
        progInst->addUniform("Texture0");
		progInst->addUniform("MatShine");
        progInst->addUniform("lightPos");
		progInst->addAttribute("vertPos");
		progInst->addAttribute("vertNor");
        progInst->addAttribute("vertTex");
		progInst->addAttribute("instanceMatrix");

  		// init splines up and down
    //    splinepath[0] = Spline(glm::vec3(-6,3,5), glm::vec3(-1,0,5), glm::vec3(1, 5, 5), glm::vec3(3,3,5), 5);
    //    splinepath[1] = Spline(glm::vec3(3,3,5), glm::vec3(4,1,5), glm::vec3(-0.75, 0.25, 5), glm::vec3(0,0,5), 5);
        splinepath[0] = Spline(glm::vec3(-20,12,-20), glm::vec3(-10,10,-10), glm::vec3(0, 8, 0), glm::vec3(10,6,10), 5);
        splinepath[1] = Spline(glm::vec3(10,6,10), glm::vec3(20,4,20), glm::vec3(25, 2, 30), glm::vec3(16,0,33), 5);
    
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
        vector<float> arms{0, PI/8.0, 0, -PI/8.0};
        for (int i=0; i < arms.size(); i++) {
            Keyframe tmp = Keyframe(arms[i], arms[i+1], frameDur);
            if (i == arms.size()-1)
               tmp = Keyframe(arms[i], arms[0], frameDur);
            lArmKF.push_back(tmp);
            rArmKF.push_back(tmp);
        }
        vector<float> legs{PI/6.0, 0, -PI/6.0, 0};
        for (int i=0; i < legs.size(); i++) {
            Keyframe tmp = Keyframe(legs[i], legs[i+1], frameDur);
            if (i == legs.size()-1)
                tmp = Keyframe(legs[i], legs[0], frameDur);
            lLegKF.push_back(tmp);

            Keyframe tmp2 = Keyframe(-1*legs[i], -1*legs[i+1], frameDur);
            if (i == legs.size()-1)
                tmp2 = Keyframe(-1*legs[i], -1*legs[0], frameDur);
            rLegKF.push_back(tmp2);
        }
        vector<float> lKnee{0, PI/3.0, 0, 0};
        for (int i=0; i < lKnee.size(); i++) {
            Keyframe tmp = Keyframe(lKnee[i], lKnee[i+1], frameDur);
            if (i == lKnee.size()-1)
                tmp = Keyframe(lKnee[i], lKnee[0], frameDur);
            lKneeKF.push_back(tmp);
        }
        vector<float> rKnee{0, 0, 0, PI/3.0};
        for (int i=0; i < rKnee.size(); i++) {
            Keyframe tmp = Keyframe(rKnee[i], rKnee[i+1], frameDur);
            if (i == rKnee.size()-1)
                tmp = Keyframe(rKnee[i], rKnee[0], frameDur);
            rKneeKF.push_back(tmp);
        }
    }

	void initGeom(const std::string& resourceDirectory)
	{
		//EXAMPLE set up to read one shape from one obj file - convert to read several
		// Initialize mesh
		// Load geometry
 		// Some obj files contain material information.We'll ignore them for this assignment.
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

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/statue/Venus_de_Milo.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			for (int i = 0; i < TOshapes.size(); i++) {
                shared_ptr<Shape> tmp = make_shared<Shape>();
                tmp->createShape(TOshapes[i]);
                tmp->measure();
                tmp->init();

                statueMesh.push_back(tmp);
            }
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
        // rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/fir/fir.obj").c_str());
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

                dummyMin.push_back(tmp->min);
                dummyMax.push_back(tmp->max);
                dummyMesh.push_back(tmp);
            }
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

        rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/road.obj").c_str());
        if (!rc) {
			cerr << errStr << endl;
		} else {
			roadObj = make_shared<Shape>();
			roadObj->createShape(TOshapes[0]);
			roadObj->measure();
			roadObj->init();
		}

        cubeMapTexture = createSky("../resources/sky/", faces);

		//code to load in the ground plane (CPU defined data passed to GPU)
		initGround();

        initKeyframes();
	}

	//directly pass quad for the ground to the GPU
	void initGround() {

		float g_groundSize = 35;
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
    		case 1: // pink
    			glUniform3f(curS->getUniform("MatAmb"), 0.063, 0.038, 0.1);
    			glUniform3f(curS->getUniform("MatDif"), 0.63, 0.38, 1.0);
    			glUniform3f(curS->getUniform("MatSpec"), 0.3, 0.2, 0.5);
    			glUniform1f(curS->getUniform("MatShine"), 4.0);
    		break;
    		case 2: 
    			glUniform3f(curS->getUniform("MatAmb"), 0.004, 0.05, 0.09);
    			glUniform3f(curS->getUniform("MatDif"), 0.04, 0.5, 0.9);
    			glUniform3f(curS->getUniform("MatSpec"), 0.02, 0.25, 0.45);
    			glUniform1f(curS->getUniform("MatShine"), 27.9);
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

   	/* code to draw waving hierarchical model */
   	void drawHierModel(shared_ptr<MatrixStack> Model, shared_ptr<Program> prog) {
   		// simplified for releaes code
		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(vec3(0, 0, -6));
			Model->scale(vec3(2.3));
			setModel(prog, Model);
			sphere->draw(prog);
		Model->popMatrix();
   	}

    void drawHouse(shared_ptr<MatrixStack> Model, shared_ptr<Program> drawProg) {
        Model->pushMatrix();
            Model->pushMatrix();
                Model->translate(vec3(0, -1.25, -7));
                Model->scale(vec3(0.25, 0.25, 0.25));

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

    void drawLamps(shared_ptr<MatrixStack> Model, shared_ptr<Program> prog) {
        Model->pushMatrix();

            float zVals[4] = {15, 9, 3, -3};
            lamp->bind(prog->getUniform("Texture0"));
            glUniform1f(prog->getUniform("MatShine"), 120);

            for (int l=0; l < 4; l++) {
                Model->pushMatrix();
                    Model->translate(vec3(4.5, -1.25, zVals[l]));
                    Model->scale(vec3(1, 1, 1));
                    setModel(prog, Model);
                    lampMesh->draw(prog);
                Model->popMatrix();
            }

            for (int r=0; r < 4; r++) {
                Model->pushMatrix();
                    Model->translate(vec3(-4.5, -1.25, zVals[r]));
                    Model->rotate(3.14159, vec3(0, 1, 0));
                    Model->scale(vec3(1, 1, 1));
                    setModel(prog, Model);
                    lampMesh->draw(prog);
                Model->popMatrix();
            }
        Model->popMatrix();
    }

    vec3 findCenter(int i) {
        float x = (dummyMesh[i]->max.x - dummyMesh[i]->min.x)/2;
        float y = (dummyMesh[i]->max.y - dummyMesh[i]->min.y)/2;
        float z = (dummyMesh[i]->max.z - dummyMesh[i]->min.z)/2;
        return vec3(x, y, z);
    }

    void handleInterpolation(Keyframe &k, float frametime, shared_ptr<MatrixStack> Model, vec3 axis, string limb) {
        // if (newFrame) {
        //     newFrame = false;
        //     k->setStart(glfwGetTime());
        //     cout << limb << endl;
        //     cout << "START: " << glfwGetTime() << endl;
        // }
        // float angle = k->interpolate(glfwGetTime());
        // if (angle != RAND_MAX) {
        //     Model->rotate(angle, axis);
        // } else {
        //     frame = (++frame)%4;
        //     newFrame = true;
        // }
        if (newFrame) {
            newFrame = false;
            k.setStart(glfwGetTime());
            cout << limb << endl;
            cout << "START: " << glfwGetTime() << endl;
        }
        float angle = k.interpolate(glfwGetTime());
        if(!k.isDone()){
            Model->rotate(angle, axis);
        } else {
            frame = (++frame)%4;
            newFrame = true;
        }
    }

    void drawDummy(shared_ptr<MatrixStack> Model, shared_ptr<Program> prog, float frametime) {
        vec3 tmp;
        //int frame = (int)(glfwGetTime()*5)%6;
        Model->pushMatrix();
            Model->translate(vec3(dummyLoc.x, dummyLoc.y, dummyLoc.z));
            //Model->translate(vec3((g_eye.x+(speed*view.x)), -1.25, (g_eye.z+(speed*view.z))));
            Model->scale(vec3(0.01, 0.01, 0.01));
            //Model->rotate(-PI/2.0, vec3(0, 1, 0));
            Model->rotate(-PI/2.0, vec3(1, 0, 0));
            setModel(prog, Model);
            for (int i=12; i <= 14; i++) {
                dummyMesh[i]->draw(prog);
            }
            dummyMesh[27]->draw(prog); // neck
            dummyMesh[28]->draw(prog); // head

            // LEFT ARM
            //  KEYFRAMES X-AXIS: -PI/2.4 (arm to side),
            //  KEYFRAMES Z-AXIS: none, PI/8.0, none, -PI/8.0
            Model->pushMatrix();
                Model->translate(vec3(1.0f*dummyMesh[21]->min.x, 1.0f*dummyMesh[21]->min.y, 1.0f*dummyMesh[21]->max.z));
                Model->rotate(-PI/2.4, vec3(1, 0, 0));
                // Model->rotate(-PI/8.0, vec3(0, 0, 1));
                // Model->rotate(lArmKF[frame], vec3(0, 0, 1));
                handleInterpolation(lArmKF[frame], frametime, Model, vec3(0, 0, 1), "Left arm");
                Model->translate(vec3(-1.0f*dummyMesh[21]->min.x, -1.0f*dummyMesh[21]->min.y, -1.0f*dummyMesh[21]->max.z));
                setModel(prog, Model);
                for (int i=21; i <=26; i++) {
                    dummyMesh[i]->draw(prog);
                }
            Model->popMatrix();
            // RIGHT ARM
            //  KEYFRAMES X-AXIS: PI/2.4 (arm to side)
            //  KEYFRAMES Z-AXIS: none, PI/8.0, none, -PI/8.0
            Model->pushMatrix();
                tmp = findCenter(15);
                Model->translate(vec3(1.0f*dummyMesh[15]->max.x, 1.0f*dummyMesh[15]->max.y, 1.0f*dummyMesh[15]->max.z));
                Model->rotate(PI/2.4, vec3(1, 0, 0));
                //Model->rotate(-PI/8.0, vec3(0, 0, 1));
                handleInterpolation(rArmKF[frame], frametime, Model, vec3(0, 0, 1), "Right arm");
                // Model->rotate(rArmKF[frame], vec3(0, 0, 1));
                Model->translate(vec3(-1.0f*dummyMesh[15]->max.x, -1.0f*dummyMesh[15]->max.y, -1.0f*dummyMesh[15]->max.z));
                setModel(prog, Model);
                for (int i=15; i <=20; i++) {
                    dummyMesh[i]->draw(prog);
                }
            Model->popMatrix();
            // LEFT LEG
            //  KEYFRAMES: PI/6.0 (leg backward), none (but knee bent), -PI/10.0, -PI/6.0, none,
            Model->pushMatrix();
                Model->translate(vec3(1.0f*dummyMesh[11]->min.x, 1.0f*dummyMesh[11]->min.y, 1.0f*dummyMesh[11]->max.z));
                // Model->rotate(lLegKF[frame], vec3(0, 1, 0));
                handleInterpolation(lLegKF[frame], frametime, Model, vec3(0, 1, 0), "Left leg");
                Model->translate(vec3(-1.0f*dummyMesh[11]->min.x, -1.0f*dummyMesh[11]->min.y, -1.0f*dummyMesh[11]->max.z));
                setModel(prog, Model);
                dummyMesh[11]->draw(prog); // pelvis
                dummyMesh[10]->draw(prog); // upper leg

                // KEYFRAMES: none, PI/3.0, PI/4.0, none, none, 
                Model->translate(vec3(1.0f*dummyMesh[9]->max.x, 1.0f*dummyMesh[9]->min.y, 1.0f*dummyMesh[9]->max.z));
                // Model->rotate(lKneeKF[frame], vec3(0, 1, 0));
                handleInterpolation(lKneeKF[frame], frametime, Model, vec3(0, 1, 0), "Left knee");
                Model->translate(vec3(-1.0f*dummyMesh[9]->max.x, -1.0f*dummyMesh[9]->min.y, -1.0f*dummyMesh[9]->max.z));
                setModel(prog, Model);
                for (int i=6; i <=9; i++) {
                    dummyMesh[i]->draw(prog);
                }
            Model->popMatrix();
            // RIGHT LEG
            //  KEYFRAMES: -PI/6.0 (leg forward), none, none, PI/6.0, none (but knee bent)
            Model->pushMatrix();
                tmp = findCenter(5);
                Model->translate(vec3(1.0f*dummyMesh[5]->max.x, 1.0f*dummyMesh[5]->max.y, 1.0f*dummyMesh[5]->max.z));
                // Model->rotate(rLegKF[frame], vec3(0, 1, 0));
                handleInterpolation(rLegKF[frame], frametime, Model, vec3(0, 1, 0), "Right leg");
                Model->translate(vec3(-1.0f*dummyMesh[5]->max.x, -1.0f*dummyMesh[5]->max.y, -1.0f*dummyMesh[5]->max.z));
                setModel(prog, Model);
                dummyMesh[5]->draw(prog); // pelvis
                dummyMesh[4]->draw(prog); // upper leg

                // KEYFRAMES: none, none, none, none, PI/3.0
                Model->translate(vec3(1.0f*dummyMesh[9]->max.x, 1.0f*dummyMesh[9]->min.y, 1.0f*dummyMesh[9]->max.z));
                // Model->rotate(rKneeKF[frame], vec3(0, 1, 0));
                handleInterpolation(rKneeKF[frame], frametime, Model, vec3(0, 1, 0), "Right knee");
                Model->translate(vec3(-1.0f*dummyMesh[9]->max.x, -1.0f*dummyMesh[9]->min.y, -1.0f*dummyMesh[9]->max.z));
                setModel(prog, Model);
                for (int i=0; i <=3; i++) {
                    dummyMesh[i]->draw(prog);
                }
            Model->popMatrix();
        Model->popMatrix();
        // vec3 tmp;
        // Model->pushMatrix();
        //     Model->loadIdentity();
        //     Model->translate(vec3(16, -1.25, 30));
        //     Model->rotate(-1.5708, vec3(1, 0, 0));
        //     Model->rotate(-1.5708, vec3(0, 0, 1));
        //     // HIPS
        //     Model->pushMatrix();
        //         Model->scale(vec3(0.01, 0.01, 0.01));
        //         setModel(prog, Model);
        //         dummyMesh[12]->draw(prog); // hips
        //     Model->popMatrix();
        //     // UPPER BODY
        //     Model->pushMatrix();
        //         Model->scale(vec3(0.01, 0.01, 0.01));
        //         setModel(prog, Model);
        //         dummyMesh[28]->draw(prog); // head
        //         dummyMesh[27]->draw(prog); // neck
        //         dummyMesh[14]->draw(prog); // torso
        //         dummyMesh[13]->draw(prog); // belly
        //     Model->popMatrix();
        //     // LEFT ARM
        //     Model->pushMatrix();
        //         //Model->rotate(-1.5708, vec3(1, 0, 0));
        //         //Model->rotate(-1.5708, vec3(0, 0, 1));
        //         Model->pushMatrix();
        //             tmp = findCenter(16);
        //             Model->translate(vec3(-tmp.x, -tmp.y, -tmp.z));
        //             // Model->rotate(-1.5708, vec3(1, 0, 0));
        //             // Model->translate(vec3((tmp.x), (tmp.y), (tmp.z)));
        //             // dummyMesh[20]->draw(prog); // hand
        //             // dummyMesh[19]->draw(prog); // wrist
        //             // dummyMesh[18]->draw(prog); // forearm
        //             // dummyMesh[17]->draw(prog); // elbow
        //             Model->scale(vec3(0.01, 0.01, 0.01));
        //             setModel(prog, Model);
        //             dummyMesh[16]->draw(prog); // bicep
        //         Model->popMatrix();

        //         Model->scale(vec3(0.01, 0.01, 0.01));
        //         setModel(prog, Model);
        //         dummyMesh[15]->draw(prog); // shoulder
        //     Model->popMatrix();
        //     // RIGHT ARM
        //     Model->pushMatrix();
        //         Model->scale(vec3(0.01, 0.01, 0.01));
        //         setModel(prog, Model);
        //         dummyMesh[26]->draw(prog); // hand
        //         dummyMesh[25]->draw(prog); // wrist
        //         dummyMesh[24]->draw(prog); // forearm
        //         dummyMesh[23]->draw(prog); // elbow
        //         dummyMesh[22]->draw(prog); // bicep
        //         dummyMesh[21]->draw(prog); // shoulder
        //     Model->popMatrix();
        //     // LEFT LEG
        //     Model->pushMatrix();
        //         Model->scale(vec3(0.01, 0.01, 0.01));
        //         setModel(prog, Model);
        //         dummyMesh[6]->draw(prog); // foot
        //         dummyMesh[7]->draw(prog); // ankle
        //         dummyMesh[8]->draw(prog); // lower leg
        //         dummyMesh[9]->draw(prog); // knee
        //         dummyMesh[10]->draw(prog); // upper leg
        //         dummyMesh[11]->draw(prog); // pelvis
        //     Model->popMatrix();
        //     // RIGHT LEG
        //     Model->pushMatrix();
        //         Model->scale(vec3(0.01, 0.01, 0.01));
        //         setModel(prog, Model);
        //         dummyMesh[0]->draw(prog); // foot
        //         dummyMesh[1]->draw(prog); // ankle
        //         dummyMesh[2]->draw(prog); // lower leg
        //         dummyMesh[3]->draw(prog); // knee
        //         dummyMesh[4]->draw(prog); // upper leg
        //         dummyMesh[5]->draw(prog); // pelvis
        //     Model->popMatrix();
        // Model->popMatrix();
    }

    void drawCar(shared_ptr<MatrixStack> Model, shared_ptr<Program> prog) {
        driveTheta = 1.5*sin(glfwGetTime());

        Model->pushMatrix();
            Model->translate(vec3(2.7, -0.85, 2));
            Model->translate(vec3(0, 0, driveTheta));
            Model->scale(vec3(0.3, 0.3, 0.3));

            setModel(prog, Model);
            float diffuse[3] = {0.840000, 0.332781, 0.311726};
            for (int i=0; i < carMesh.size(); i++) {
                int mat = carMesh[i]->getMat()[0];
                SetGenericMat(prog, carMat[mat].ambient, carMat[mat].diffuse, carMat[mat].specular, carMat[mat].shininess, "car");
                carMesh[i]->draw(prog);
            }
        Model->popMatrix();
    }

    void drawRoad(shared_ptr<MatrixStack> Model, shared_ptr<Program> prog) {
        Model->pushMatrix();
            Model->pushMatrix();
                Model->translate(vec3(0, -1.24, 5));
                Model->rotate(3.1416, vec3(0, 0, 1));
                Model->scale(vec3(1.9, 0.1, 3));

                glUniform1i(prog->getUniform("flip"), 0);
                road->bind(prog->getUniform("Texture0"));
                setModel(prog, Model);
                roadObj->draw(prog);
            Model->popMatrix();
        Model->popMatrix();
    }

   	void updateUsingCameraPath(float frametime)  {

   	  if (goCamera) {
        g_lookAt = vec3(16, 0, 30);
        if(!splinepath[0].isDone()){
       		splinepath[0].update(frametime);
            g_eye = splinepath[0].getPosition();
        } else {
            splinepath[1].update(frametime);
            g_eye = splinepath[1].getPosition();
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
                        numWalls++;
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

    void launchOverview(float frametime) {
        splinepath[0] = Spline(glm::vec3(-20,12,-20), glm::vec3(-10,10,-10), glm::vec3(0, 8, 0), glm::vec3(10,6,10), 5);
        splinepath[1] = Spline(glm::vec3(10,6,10), glm::vec3(20,4,20), glm::vec3(25, 2, 30), glm::vec3(16,0,33), 5);
        g_lookAt = vec3(16, 0, 30);
        if(!splinepath[0].isDone()){
       		splinepath[0].update(frametime);
            g_eye = splinepath[0].getPosition();
        } else {
            splinepath[1].update(frametime);
            g_eye = splinepath[1].getPosition();
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
		updateUsingCameraPath(frametime);
        // if (setLaunch) {
        //     launchOverview(frametime);
        //     setLaunch = false;
        // }

		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 100.0f);

		texProg->bind();
            glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
            SetView(texProg);
            // glUniform3f(texProg->getUniform("lightPos"), 3.0+lightTrans, 8.0, 7);
            glUniform3f(texProg->getUniform("lightPos"), g_eye.x-0.5, g_eye.y, g_eye.z-0.5);
            glUniform1i(texProg->getUniform("flip"), 1);

            // Model->pushMatrix();
            //     Model->translate(vec3(15, -1.25, 31));
            //     Model->scale(vec3(0.5, 0.5, 0.5));
            //     statueTex->bind(texProg->getUniform("Texture0"));
            //     glUniformMatrix4fv(texProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
            //     for (int i=0; i < statueMesh.size(); i++) {
            //         statueMesh[i]->draw(texProg);
            //     }
            // Model->popMatrix();

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

            // drawHouse(Model, texProg);
            // drawLamps(Model, texProg);
            // drawRoad(Model, texProg);

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
            glUniform3f(progInst->getUniform("lightPos"), g_eye.x-0.5, g_eye.y, g_eye.z-0.5);
            for (int i=0; i < cubeInst.size(); i++) {   
                int mat = cubeInst[i]->getMat()[0];
                SetGenericMat(progInst, treeMat[mat].ambient, treeMat[mat].diffuse, treeMat[mat].specular, treeMat[mat].shininess, "tree");
                if (treeMat[mat].diffuse_texname != "") {
                    textureMap.at(treeMat[mat].diffuse_texname)->bind(progInst->getUniform("Texture0"));
                }
                // if (i==0) {
                //     firTrunk->bind(progInst->getUniform("Texture0"));
                // } else {
                //     firLeaf->bind(progInst->getUniform("Texture0"));
                // }
                cubeInst[i]->draw(progInst);
            }
		progInst->unbind();

        //to draw the sky box bind the right shader
        cubeProg->bind();
            //set the projection matrix - can use the same one
            glUniformMatrix4fv(cubeProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
            //set the depth function to always draw the box!
            glDepthFunc(GL_LEQUAL);
            //set up view matrix to include your view transforms
            //(your code likely will be different depending
            SetView(cubeProg);
            //set and send model transforms - likely want a bigger cube
            Model->pushMatrix();
            Model->translate(vec3(0, 0, 0));
            Model->rotate(3.1416, vec3(0, 1, 0));
            Model->scale(vec3(70, 70, 70));
            glUniformMatrix4fv(cubeProg->getUniform("M"), 1, GL_FALSE,value_ptr(Model->topMatrix()));
            Model->popMatrix();
            //bind the cube map texture
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);

            //draw the actual cube
            cube->draw(cubeProg);

            //set the depth test back to normal!
            glDepthFunc(GL_LESS);
            //unbind the shader for the skybox
        cubeProg->unbind(); 

        prog->bind();
            glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
            glUniform3f(prog->getUniform("lightPos"), g_eye.x-0.5, g_eye.y, g_eye.z-0.5);
            SetView(prog);
            SetMaterial(prog, 2);
            drawDummy(Model, prog, frametime);
        prog->unbind();

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

	// Quit program.
	windowManager->shutdown();
	return 0;
}
