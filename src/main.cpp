#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <limits>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

//using namespace std;

int width = 1900, height = 1080;	// Default window size
GLFWwindow *window = nullptr;	// Our window object
double cx = 0.0, cy = 0.0, zoom = 0.4; // Camera position and zoom value
int itr = 100;	// Number of iterations

int main(int argc, char **argv)
{
	if(!glfwInit()) {
		std::cerr << "Failed to init GLFW!\n";
		return 1;
	}
	atexit(glfwTerminate); // i dont get this
	
	// Set error callback function
	glfwSetErrorCallback( [](int e, const char* s) {std::cerr << s << std::endl;} );
	
	// OpenGL 4.5 .... we need double precision on GPU
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	window = glfwCreateWindow(width, height, "Mandelbrot", nullptr, nullptr);
	if(!window) {
		std::cerr << "Failed to create window." << std::endl;
		return 1;
	}
	
	// Lets us do OpenGL stuff
	glfwMakeContextCurrent(window);
	
	// Init GLEW
	glewExperimental = GL_TRUE;
	glewInit();
	
	std::cout << "Renderer: " << glGetString((GL_RENDERER)) << std::endl;
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
	
	// Shaders
	const char* vertexShader =
		"#version 410\n"
		"in vec3 vp;"
		"void main() {"
		"	gl_Position = vec4(vp, 0.5);"
		"}";
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertexShader, nullptr);
	glCompileShader(vs);
	
	std::ifstream t("fragmentShader.fs");
	if(!t.is_open()) {
		std::cerr << "Could not open fragment shader!\n";
		return 2;
	}
	std::string str((std::istreambuf_iterator<char>(t)),
					 std::istreambuf_iterator<char>());
	const char* src = str.c_str();
	
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &src, nullptr);
	glCompileShader(fs);
	
	int success;
	glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
	if(!success) {
		int s;
		glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &s);
		char *buf = new char[s];
		glGetShaderInfoLog(fs, s, &s, buf);
		std::cerr << buf << std::endl;
		delete[] buf;
		return 2;
	}
	
	// Create program
	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, fs);
	glAttachShader(shader_program, vs);
	glLinkProgram(shader_program);
	
	
	// Big rectangle
	float points[] = {
		-1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		
		-1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
	};
	
	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 2*9*sizeof(float), points, GL_STATIC_DRAW);
	
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	
	// Use our shader program
	glUseProgram(shader_program);
	glBindVertexArray(vao);
	
	while(!glfwWindowShouldClose(window)) {
		glfwGetWindowSize(window, &width, &height);
		glUniform2d(glGetUniformLocation(shader_program, "screen_size"), (double)width, (double)height);
		glUniform1d(glGetUniformLocation(shader_program, "screen_ratio"), (double)width / (double)height);
		glUniform2d(glGetUniformLocation(shader_program, "center"), cx,cy);
		glUniform1d(glGetUniformLocation(shader_program, "zoom"), zoom);
		glUniform1i(glGetUniformLocation(shader_program, "itr"), itr);
		
		// Clear screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		glfwSwapBuffers(window); // Update the screen
		glfwPollEvents(); // Polls for events
	}
	glfwDestroyWindow(window);
	
	return 0;
}
