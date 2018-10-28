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
bool keys[1024] = {0};
int fps = 0;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	const double d = 0.1/zoom; // wooosh magic?
	
	// Store which keys were pressed
	if(action == GLFW_PRESS) {
		keys[key] = 1;
	} else if(action == GLFW_RELEASE) {
		keys[key] = 0;
	}
	
	if(keys[GLFW_KEY_ESCAPE]) {
		glfwSetWindowShouldClose(window, 1);
	} else if(keys[GLFW_KEY_A]) {
		cx -= d;
	} else if(keys[GLFW_KEY_D]) {
		cx += d;
	} else if(keys[GLFW_KEY_W]) {
		cy += d;
	} else if(keys[GLFW_KEY_S]) {
		cy -= d;
	} else if(keys[GLFW_KEY_KP_ADD] && itr<std::numeric_limits<int>::max() - 10) {
		// how does this condition work?
		itr += 10;
	} else if(keys[GLFW_KEY_KP_SUBTRACT]) {
		itr -= 10;
		if(itr <= 0)
			itr = 0;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	zoom += yoffset * 0.1 * zoom;
	if(zoom < 0.1) {
		zoom = 0.1;
	}
}

void updateWindowTitle() {
	std::ostringstream ss;
	ss << "Mandelbrot, " << "FPS: " << fps;
	ss << ", Iterations: " << itr;
	ss << ", Zoom: " << zoom;
	ss << ", At: (" << std::setprecision(8) << cx << " + " << cy << "i)";
	glfwSetWindowTitle(window, ss.str().c_str());
}

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
	
	// User input
	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);
	
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
	
	// FPS
	time_t lastTime = glfwGetTime();
	int ticks = 0;
	
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
		
		ticks++;
		time_t currentTime = glfwGetTime();
		if(currentTime - lastTime > 1.0) {
			fps = ticks;
			lastTime = currentTime;
			updateWindowTitle();
			ticks = 0;
		}
	}
	glfwDestroyWindow(window);
	
	return 0;
}
