/* Lab 6 base code - transforms using local matrix functions
	to be written by students -
	based on lab 5 by CPE 471 Cal Poly Z. Wood + S. Sueda
	& Ian Dunn, Christian Eckhardt
*/
#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "camera.h"
// used for helper in perspective
#include "glm/glm.hpp"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;

#define RADIUS 0.8
#define CYL_HEIGHT 1.0
#define TRI 90

class cylinder
{
public:
    GLfloat circle_vertices[3*(TRI+1)], cylinder_vertices[2*3*TRI +6];
    GLushort circle_indices[TRI*3], cylinder_indices[TRI*2*3+ 2*TRI*3];
    GLfloat cylinder_colors[2*3*TRI +6];
    
    cylinder(){
        buildCircleVertices();
        buildCircleIndices();
        buildCylinderVertices();
        buildCylinderIndices();
        buildCylinderColors();
    }
    
    void buildCylinderColors(){
        int k;
        for(k=0; k<sizeof(cylinder_colors)/sizeof(GLfloat); k++){
            cylinder_colors[k] = cylinder_vertices[k];
        }
        

    }
    
    void buildCircleVertices(){
        
        int counter = 0;
        
        circle_vertices[0] = 0.0;
        circle_vertices[1] = 0.0;
        circle_vertices[2] = 0.0;
        
        for(int i=1; i<3*(TRI+1);i++){
            
            
            circle_vertices[i*3] = paraXCoord(counter*(360/TRI)/2,RADIUS);
            circle_vertices[i*3+1] = paraYCoord(counter*(360/TRI)/2,RADIUS);
            circle_vertices[i*3+2] = 0.0;
            
            counter-=2;
        }
        
        
    }
    
    void buildCircleIndices(){
        for(int i=0; i<(TRI*3)-1; i++){
            circle_indices[i*3] = i+1;
            circle_indices[i*3+1] = 0;
            circle_indices[i*3+2] = i+2;
        }
        circle_indices[TRI*3-1] = circle_indices[0];
    }
    
    void buildCylinderVertices(){
        int a;
        for(a=0; a<TRI; a++){
            cylinder_vertices[a*6] = circle_vertices[(a+1)*3];
            cylinder_vertices[a*6+1] = circle_vertices[(a+1)*3+1];
            cylinder_vertices[a*6+2] = CYL_HEIGHT/2;
            
            cylinder_vertices[a*6+3] = circle_vertices[(a+1)*3];
            cylinder_vertices[a*6+4] = circle_vertices[(a+1)*3+1];
            cylinder_vertices[a*6+5] = -CYL_HEIGHT/2;
            
        }
        
        cylinder_vertices[a*6] = circle_vertices[0];
        cylinder_vertices[a*6+1] = circle_vertices[1];
        cylinder_vertices[a*6+2] = CYL_HEIGHT/2;
        cylinder_vertices[a*6+3] = circle_vertices[0];
        cylinder_vertices[a*6+4] = circle_vertices[1];
        cylinder_vertices[a*6+5] = -CYL_HEIGHT/2;
    }
    
    void buildCylinderIndices(){
        //build cylinder
        int x;

        for(x=1; x<TRI*2-1; x+=2){
            cylinder_indices[x*3] = x;
            cylinder_indices[x*3+1] = x+1;
            cylinder_indices[x*3+2] = x+2;
        }
        for(x=0; x<TRI*2-1; x+=2){
            cylinder_indices[x*3] = x;
            cylinder_indices[x*3+1] = x+2;
            cylinder_indices[x*3+2] = x+1;
        }

        cylinder_indices[(2*3*TRI)-6] = x-2;
        cylinder_indices[(2*3*TRI)-5] = 0;
        cylinder_indices[(2*3*TRI)-4] = --x;
        cylinder_indices[(2*3*TRI)-3] = x++;
        cylinder_indices[(2*3*TRI)-2] = 0;
        cylinder_indices[(2*3*TRI)-1] = 1;
        
        static int m = x;
        int y;

        //back circle
        for(y=0; x<TRI*3-1; y++){
            cylinder_indices[x*3] = 2*y +3;
            cylinder_indices[x*3+1] = TRI*2 +1;
            cylinder_indices[x*3+2] = 2*y +1;
            x++;
        }
        cylinder_indices[x*3] = cylinder_indices[m*3+2];
        cylinder_indices[x*3+1] = TRI*2 +1;
        cylinder_indices[x*3+2] = 2*y+1;

        //front circle
        m=++x;
        for(y=0; x<TRI*4-1; y++){
            cylinder_indices[x*3] = 2*y;
            cylinder_indices[x*3+1] = TRI*2;
            cylinder_indices[x*3+2] = 2*y+2;
            x++;
        }
        cylinder_indices[x*3] = 2*y;
        cylinder_indices[x*3+1] = TRI*2;
        cylinder_indices[x*3+2] = cylinder_indices[m*3];
        
    }


    float paraXCoord(float angle, float radius){
        return radius*cos(angle*M_PI/180);
    }
    
    float paraYCoord(float angle, float radius){
        return radius*sin(angle*M_PI/180);
    }

};




class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog, prog2;

	// Shape to be used (from obj file)
	shared_ptr<Shape> shape;
	
	//camera
	camera mycam;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;
    GLuint VertexArrayID1;


	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;
    GLuint VertexBufferID1;
    GLuint VertexColorID;
    GLuint IndexBufferID1;
    
    cylinder K;
    
    float walk_val=0, flap_ears;
    int count=0;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}
        if(key == GLFW_KEY_SPACE && action == GLFW_PRESS){
            if(count%2==0)
                walk_val = 1;
            else
                walk_val = 0;
            count++;
        }
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

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.12f, 0.34f, 0.56f, 1.0f);

		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		//culling:
        glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);

		//transparency
        glEnable(GL_BLEND);
		//next function defines how to mix the background color with the transparent pixel in the foreground.
		//This is the standard:
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		if (! prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->init();
		prog->addUniform("P");
		prog->addUniform("MV");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
        
        
        
        prog2 = make_shared<Program>();
        prog2->setVerbose(true);
        prog2->setShaderNames(resourceDirectory + "/simple_vert33.glsl", resourceDirectory + "/simple_frag33.glsl");
        if (! prog2->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        prog2->init();
        prog2->addUniform("P");
        prog2->addUniform("MV");
        prog2->addAttribute("vertPos");
        prog2->addAttribute("vertNor");
        
        
	}

	void initSphere(const std::string& resourceDirectory)
	{
		// Initialize mesh.
		shape = make_shared<Shape>();
		shape->loadMesh(resourceDirectory + "/IcoSphere.obj");
		shape->resize();
		shape->init();
		
	}
    
    void initCylinder(const std::string& resourceDirectory)
    {
        glGenVertexArrays(1, &VertexArrayID1);
        glBindVertexArray(VertexArrayID1);
        
        glGenBuffers(1, &VertexBufferID1);
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID1);
        glBufferData(GL_ARRAY_BUFFER, sizeof(K.cylinder_vertices), K.cylinder_vertices, GL_DYNAMIC_DRAW);
        
        
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
        
        glGenBuffers(1, &VertexColorID);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, VertexColorID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(K.cylinder_colors), K.cylinder_colors, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        glGenBuffers(1, &IndexBufferID1);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferID1);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(K.cylinder_indices), K.cylinder_indices, GL_STATIC_DRAW);
        
        glBindVertexArray(0);
    }
    
    void drawSphere(std::__1::shared_ptr<MatrixStack> MV){
        glUniformMatrix4fv(prog2->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
        shape->draw(prog2);
        MV->popMatrix();
    }
    
    void drawCylinder(std::__1::shared_ptr<MatrixStack> MV){
        glBindVertexArray(VertexArrayID1);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferID1);
        glUniformMatrix4fv(prog2->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
        glDrawElements(GL_TRIANGLES, sizeof(K.cylinder_indices)/sizeof(GLushort), GL_UNSIGNED_SHORT, (void*)0);
        MV->popMatrix();
    }
    
    
    void drawTrunk(std::__1::shared_ptr<MatrixStack> MV){
        MV->pushMatrix();
        MV->translate(glm::vec3(0, -.3, 1.2));
        MV->rotate(0.6, glm::vec3(1,0,0));
        MV->scale(glm::vec3(0.2, 0.2, 1));
        drawCylinder(MV);
    }
    
    int hit1=0;
    void drawEars(std::__1::shared_ptr<MatrixStack> MV){
        static float a = 0;
        if(walk_val==1){
            if(a<=-0.5236)
                hit1 = 1;
            if(a>=0)
                hit1 = 0;
            if(hit1==1)
                a+=0.005;
            else if(hit1==0 && a>-0.5236)
                a-=0.005;
        }
        
        MV->pushMatrix();
        MV->translate(glm::vec3(-0.8, 0, 0));
        MV->rotate(a,glm::vec3(0,1,0));
        MV->scale(glm::vec3(0.7, 1, 0.1));
        
        drawSphere(MV);
        
        //ears
        MV->pushMatrix();
        MV->translate(glm::vec3(0.8, 0, 0));
        MV->rotate(-a,glm::vec3(0,1,0));
        MV->scale(glm::vec3(0.7, 1, 0.1));
        
        drawSphere(MV);
    }
    void drawHead(std::__1::shared_ptr<MatrixStack> MV){
        MV->pushMatrix();
        MV->translate(glm::vec3(0,0.2,1.0));
        MV->scale(glm::vec3(0.9,0.6,.5));
    }
    
    int hit =0;
    void drawLegs(std::__1::shared_ptr<MatrixStack> MV){
        static float a = M_PI/2;
        if(walk_val==1){
            if(a<=1.0472)
                hit = 1;
            if(a>=M_PI/2)
                hit = 0;
            if(hit==1)
                a+=0.005;
            else if(hit==0 && a>1.0472)
                a-=0.005;
        }
        
        //leftfront
        MV->pushMatrix();
        MV->translate(glm::vec3(-.6, -0.7,0.4));
        MV->rotate(a,glm::vec3(1,0,0));
        MV->scale(glm::vec3(0.36,0.24,0.6));
        drawCylinder(MV);
        
        //rightfront
        MV->pushMatrix();
        MV->translate(glm::vec3(.6, -0.7,0.4));
        MV->rotate(-a,glm::vec3(1,0,0));
        MV->scale(glm::vec3(0.36,0.24,0.6));
        drawCylinder(MV);

        
        //left back
        MV->pushMatrix();
        MV->translate(glm::vec3(-.6, -0.7,-0.4));
        MV->rotate(a,glm::vec3(1,0,0));
        MV->scale(glm::vec3(0.36,0.24,0.6));
        drawCylinder(MV);

        
        //right back
        MV->pushMatrix();
        MV->translate(glm::vec3(.6, -0.7,-0.4));
        MV->rotate(-a,glm::vec3(1,0,0));
        MV->scale(glm::vec3(0.36,0.24,0.6));
        drawCylinder(MV);

        
    }
    
    void drawBody(std::__1::shared_ptr<MatrixStack> MV){
        MV->pushMatrix();
        MV->scale(glm::vec3(0.5,0.75,0.9));
        MV->scale(glm::vec3(0.5,0.5,0.5));
    }
    
    
    void drawElephant(std::__1::shared_ptr<MatrixStack> MV){
        drawBody(MV);
            drawHead(MV);
                drawEars(MV);
                drawTrunk(MV);
            drawSphere(MV);
            drawLegs(MV);
        drawSphere(MV);
    }
    
    void drawSnowGlobe(std::__1::shared_ptr<MatrixStack> MV, std::__1::shared_ptr<MatrixStack> P){
        MV->pushMatrix();
        glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
        
        glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
        glFrontFace(GL_CW);
        shape->draw(prog);
        glFrontFace(GL_CCW);
    }
    

	void render()
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		auto P = std::make_shared<MatrixStack>();
		auto MV = std::make_shared<MatrixStack>();
		P->pushMatrix();	
		P->perspective(70., width, height, 0.1, 100.0f);
		

		MV->pushMatrix();
		MV->translate(glm::vec3(0,0,-2.5));
        glm::mat4 V = mycam.process();
        
        MV->multMatrix(V);
		

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        prog2->bind();

        glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
        drawElephant(MV);

        prog2->unbind();
        
        
        // Draw mesh using GLSL
        prog->bind();
        
        drawSnowGlobe(MV, P);

        prog->unbind();
        
	}
};
//*********************************************************************************************************
int main(int argc, char **argv)
{
	// Where the resources are loaded from
	std::string resourceDir = "../../resources";

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

	application->init(resourceDir);
	application->initSphere(resourceDir);
    application->initCylinder(resourceDir);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
