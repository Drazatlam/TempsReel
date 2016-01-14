#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include "maillage.h"

int testColor=0;
int testPos1=0;
int testPos2=0;
int cam=0;
glm::mat4x4 projectionMat;
glm::mat4x4 lightMat;
glm::mat4x4 viewMat;

GLuint buffer;
float datas[]={-0.5,-0.5,0,0.5,-0.5,0,0.5,0.5,0,-0.5,0.5,0};
Maillage m;

void render(GLFWwindow*);
void init();
void testBuffer();
void bufferObj(Maillage m);

#define glInfo(a) std::cout << #a << ": " << glGetString(a) << std::endl

// This function is called on any openGL API error
void debug(GLenum, // source
		   GLenum, // type
		   GLuint, // id
		   GLenum, // severity
		   GLsizei, // length
		   const GLchar *message,
		   const void *) // userParam
{
	std::cout << "DEBUG: " << message << std::endl;
}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
	{
		std::cerr << "Could not init glfw" << std::endl;
		return -1;
	}

	// This is a debug context, this is slow, but debugs, which is interesting
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	if (!window)
	{
		std::cerr << "Could not init window" << std::endl;
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	GLenum err = glewInit();
	if(err != GLEW_OK)
	{
		std::cerr << "Could not init GLEW" << std::endl;
		std::cerr << glewGetErrorString(err) << std::endl;
		glfwTerminate();
		return -1;
	}

	// Now that the context is initialised, print some informations
	glInfo(GL_VENDOR);
	glInfo(GL_RENDERER);
	glInfo(GL_VERSION);
	glInfo(GL_SHADING_LANGUAGE_VERSION);

	// And enable debug
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

	//glDebugMessageCallback((GLDEBUGPROC)debug, nullptr);

	// This is our openGL init function which creates ressources
	init();

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		render(window);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

// Build a shader from a string
GLuint buildShader(GLenum const shaderType, std::string const src)
{
	GLuint shader = glCreateShader(shaderType);

	const char* ptr = src.c_str();
	GLint length = src.length();

	glShaderSource(shader, 1, &ptr, &length);

	glCompileShader(shader);

	GLint res;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &res);
	if(!res)
	{
		std::cerr << "shader compilation error" << std::endl;

		char message[1000];

		GLsizei readSize;
		glGetShaderInfoLog(shader, 1000, &readSize, message);
		message[999] = '\0';

		std::cerr << message << std::endl;

		glfwTerminate();
		exit(-1);
	}

	return shader;
}

// read a file content into a string
std::string fileGetContents(const std::string path)
{
	std::ifstream t(path);
	std::stringstream buffer;
	buffer << t.rdbuf();

	return buffer.str();
}

// build a program with a vertex shader and a fragment shader
GLuint buildProgram(const std::string vertexFile, const std::string fragmentFile)
{
	auto vshader = buildShader(GL_VERTEX_SHADER, fileGetContents(vertexFile));
	auto fshader = buildShader(GL_FRAGMENT_SHADER, fileGetContents(fragmentFile));

	GLuint program = glCreateProgram();

	glAttachShader(program, vshader);
	glAttachShader(program, fshader);

	glLinkProgram(program);

	GLint res;
	glGetProgramiv(program, GL_LINK_STATUS, &res);
	if(!res)
	{
		std::cerr << "program link error" << std::endl;

		char message[1000];

		GLsizei readSize;
		glGetProgramInfoLog(program, 1000, &readSize, message);
		message[999] = '\0';

		std::cerr << message << std::endl;

		glfwTerminate();
		exit(-1);
	}

	return program;
}

/****************************************************************
 ******* INTERESTING STUFFS HERE ********************************
 ***************************************************************/

// Store the global state of your program
struct
{
	GLuint program; // a shader
	GLuint vao; // a vertex array object
	GLuint depthTexture;
	GLuint fbo;
} gs;

void init()
{
	// Build our program and an empty VAO
	gs.program = buildProgram("basic.vsl", "basic.fsl");

	glm::vec3 min;
	glm::vec3 max;
	m.geometry(glm::vec3(0.f,0.f,0.f),"C:/Users/etu/Desktop/bestMonkey.obj",min,max);

	bufferObj(m);

	glGenTextures(1,&gs.depthTexture);
	glBindTexture(GL_TEXTURE_2D, gs.depthTexture);
	glTexStorage2D(GL_TEXTURE_2D,1,GL_DEPTH_COMPONENT32F, 800, 800);


	glGenFramebuffers(1, &gs.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, gs.fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,gs.depthTexture,0);

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	glBindFramebuffer(GL_FRAMEBUFFER,0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gs.depthTexture);
	//testBuffer();
}

void bufferObj(Maillage m){
	std::vector<float> points=m.points();
	std::vector<float> normals=m.normals();

	std::vector<float> all;
	all.reserve(points.size()*2);
	for(int i=0;i<points.size();i+=3){
		all.push_back(points[i]);
		all.push_back(points[i+1]);
		all.push_back(points[i+2]);
		all.push_back(normals[i]);
		all.push_back(normals[i+1]);
		all.push_back(normals[i+2]);
	}

	glGenBuffers(1,&buffer);
	glBindBuffer(GL_ARRAY_BUFFER,buffer);


	glBufferData(GL_ARRAY_BUFFER,all.size()*4,all.data(),GL_STATIC_DRAW);
	std::cout<<"ok"<<std::endl;
	glCreateVertexArrays(1,&gs.vao);
	glBindVertexArray(gs.vao);
	{
		glBindBuffer(GL_ARRAY_BUFFER,buffer);
		glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*4,0);
		glVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE,6*4,(void*)(3*4));
		glEnableVertexArrayAttrib(gs.vao,1);
		glEnableVertexArrayAttrib(gs.vao,3);
	}
	glBindVertexArray(0);
}


void testBuffer(){
	glGenBuffers(1,&buffer);
	glBindBuffer(GL_ARRAY_BUFFER,buffer);
	glBufferData(GL_ARRAY_BUFFER,3*4*4,datas,GL_STATIC_DRAW);

	glCreateVertexArrays(1,&gs.vao);
	glBindVertexArray(gs.vao);
	{
		glBindBuffer(GL_ARRAY_BUFFER,buffer);
		glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,0);
		glEnableVertexArrayAttrib(gs.vao,1);
	}
	glBindVertexArray(0);
}
void pass(GLuint buffer,int width,int height,glm::mat4x4 viewMat){

	glBindFramebuffer(GL_FRAMEBUFFER,buffer);
	glViewport(0, 0, width, height);

	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(gs.program);
	glBindVertexArray(gs.vao);
	glEnable(GL_DEPTH_TEST);

	projectionMat= glm::perspective(45.0f,640.0f/480.0f,1.0f,200.0f);

	glm::mat4x4 mat=projectionMat*viewMat;

	glProgramUniformMatrix4fv(gs.program,2,1,GL_FALSE,&mat[0][0]);

	glProgramUniform3f(gs.program,10,0.5,-2,-2);

	glDrawArrays(GL_TRIANGLES, 0, m.topo.size()*3);

	//glDrawElements

	glBindVertexArray(0);
	glUseProgram(0);
}
void render(GLFWwindow* window)
{
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	++cam;
	//viewMat=glm::lookAt(glm::vec3(cos(cam/100.0f)*5,-5,sin(cam/100.0f)*5),glm::vec3(0,0,0),glm::vec3(0,-1,0));

	//lightMat=glm::lookAt(glm::vec3(cos(cam/100.0f)*2,0,-sin(cam/100.0f)*2),glm::vec3(0,0,0),glm::vec3(0,-1,0));

	lightMat=glm::lookAt(glm::vec3(4*cos(cam/100.0f),-2.5*sin(cam/100.0f)-2.5,-5),glm::vec3(0,0,0),glm::vec3(0,-1,0));

	viewMat=glm::lookAt(glm::vec3(0,-2,-12),glm::vec3(0,0,0),glm::vec3(0,-1,0));


	projectionMat= glm::perspective(45.0f,640.0f/480.0f,1.0f,200.0f);
	glm::mat4x4 mat=projectionMat*lightMat;

	glProgramUniformMatrix4fv(gs.program,8,1,GL_FALSE,&mat[0][0]);


	pass(gs.fbo,800,800,lightMat);

	pass(0,width,height,viewMat);








}