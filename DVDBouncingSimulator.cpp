#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
glm::vec3 moveDVDSprite(float dt, unsigned int window_width);
glm::vec3 getRandomColor();
float randomColor();
namespace {
	//settings
	const unsigned int SCR_WIDTH = 800;
	const unsigned int SCR_HEIGHT = 600;

	unsigned int shaderProgram;
	unsigned int shaderProgram2;

	glm::vec3 dvd_position;
	glm::vec3 dvd_velocity(0.0008f, -0.0002f, 0.0f);
	//glm::vec2 dvd_size(1.0f, 1.0f);


	// Calcular el ancho
	float width = (0.125f - (-0.125f)); // Ancho como float, resultando en 0.25f

	// Crear un glm::vec3 con el ancho
	glm::vec3 dvd_size = glm::vec3(width, 0.0f, 0.0f); // Ancho en el eje X, cero en Y y Z

	const char* vertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"layout (location = 1) in vec3 aColor;\n"
		"layout (location = 2) in vec2 aTexCoord;\n"
		"out vec3 ourColor;\n"
		"out vec2 TexCoord;\n"
		"uniform mat4 transform;"
		"void main()\n"
		"{\n"
		"   gl_Position = transform * vec4(aPos, 1.0);\n"
		"	ourColor = aColor;\n"
		"   TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
		"}\0";
	const char* fragmentShaderSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec3 ourColor;\n"
		"in vec2 TexCoord;\n"
		"uniform sampler2D ourTexture;\n"
		//"vec3 colorB = vec3(1.000,0.833,0.224);\n"
		"uniform vec3 colorB;\n"
		"void main()\n"
		"{\n"
		"   FragColor = texture(ourTexture, TexCoord)* vec4(colorB, 1.0);\n"
		"}\n\0";

	const char* fragmentShaderColorSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec3 ourColor;\n"
		"in vec2 TexCoord;\n"
		"float rand(vec2 co){ return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453); }\n"
		"float rnd(float i) {return mod(4000.*sin(23464.345*i+45.345),1.);}\n"
		"float r1 = rand(TexCoord);\n"
		"float r2 = rand(TexCoord);\n"
		"float r3 = rand(TexCoord);\n"
		"vec3 noise = vec3(r1,r2,0.912);\n"
		"vec3 colorA = vec3(0.149,0.141,0.912);\n"
		"uniform sampler2D ourTexture;\n"
		"void main()\n"
		"{\n"
		"   FragColor = texture(ourTexture, TexCoord) * vec4(noise, 1.0);\n"
		"}\n\0";
}

int main(void)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

#ifndef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif // !__APPLE__

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "DVD Bouncing Simulator", NULL, NULL);
	if (!window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		
	int window_width, window_height;
	glfwGetFramebufferSize(window, &window_width, &window_height);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	unsigned int vertexShader2 = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader2, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader2);

	int success2;
	char infoLog2[512];
	glGetShaderiv(vertexShader2, GL_COMPILE_STATUS, &success2);
	if (!success2)
	{
		glGetShaderInfoLog(vertexShader2, 512, NULL, infoLog2);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog2 << std::endl;
	}

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}


	unsigned int fragmentColorShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentColorShader, 1, &fragmentShaderColorSource, NULL);
	glCompileShader(fragmentColorShader);
	glGetShaderiv(fragmentColorShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentColorShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	shaderProgram = glCreateProgram();
	shaderProgram2 = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glAttachShader(shaderProgram2, vertexShader2);
	glAttachShader(shaderProgram2, fragmentColorShader);
	glLinkProgram(shaderProgram);
	glLinkProgram(shaderProgram2);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteShader(fragmentColorShader);

	float vertices[] = {
		// positions              // colors              // texture cords
		 0.125f,  0.125f, -0.25f, 1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
		 0.125f, -0.125f, -0.25f, 0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
		-0.125f, -0.125f, -0.25f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
		-0.125f,  0.125f, -0.25f, 1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left

	};
	unsigned int indices[] = {
		0,1,3,  //First triangle
		1,2,3   // second triangle
	};

	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load("resources/textures/logo.png", &width, &height, &nrChannels, 0);

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	glfwSetTime(3.29149);

	glUseProgram(shaderProgram);
	glm::vec3 randomColor = getRandomColor();
	glUniform3f(glGetUniformLocation(shaderProgram, "colorB"), randomColor.r, randomColor.g, randomColor.b);
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		glBindVertexArray(VAO);

		glBindTexture(GL_TEXTURE_2D, texture);

		//transforms
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, moveDVDSprite((float)glfwGetTime(), SCR_WIDTH));
		//transform = glm::rotate(transform, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
		//glUseProgram(shaderProgram);
		//glUseProgram(shaderProgram2);
		unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
		/*glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.04f, 0.0f));*/
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		/* Swap front and back buffers */
		glfwSwapBuffers(window);
		/* Poll for and process events */
		glfwPollEvents();
	}
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteProgram(shaderProgram);

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

glm::vec3 moveDVDSprite(float dt, unsigned int window_width)
{
	const float x_max = 0.917626f;
	const float x_min = 0.817626f;
	const float y_max = 0.90f;
	const float y_min = -y_max;
	dvd_position += dvd_velocity * dt * 0.20f;

	//std::cout << "x: " << dvd_position.x << " y: " << dvd_position.y << std::endl;
	//std::cout << "x: " << dvd_velocity.x << " y: " << dvd_velocity.y << std::endl;
	//std::cout << "delta: " << dt << std::endl;

	if (dt >= 3.29149f) {
		glfwSetTime(3.29149);
		dt = 3.29149;
	}
	
	if (dvd_position.x <= -x_max) {
		dvd_velocity.x = -dvd_velocity.x;
		dvd_position.x = -x_max;
		glUseProgram(shaderProgram);
		glm::vec3 randomColor = getRandomColor();
		glUniform3f(glGetUniformLocation(shaderProgram, "colorB"), randomColor.r, randomColor.g, randomColor.b);
		std::cout << "x: " << dvd_position.x << " y: " << dvd_position.y << std::endl;
	}
	else if (dvd_position.x  >= x_max) {
		dvd_velocity.x = -dvd_velocity.x;
		dvd_position.x = x_max;
		glUseProgram(shaderProgram);
		glm::vec3 randomColor = getRandomColor();
		glUniform3f(glGetUniformLocation(shaderProgram, "colorB"), randomColor.r, randomColor.g, randomColor.b);
		std::cout << "x: " << dvd_position.x << " y: " << dvd_position.y << std::endl;
	}
	
	if (dvd_position.y >= y_max) {
		dvd_velocity.y = -dvd_velocity.y;
		dvd_position.y = y_max;
		glUseProgram(shaderProgram);
		glm::vec3 randomColor = getRandomColor();
		glUniform3f(glGetUniformLocation(shaderProgram, "colorB"), randomColor.r, randomColor.g, randomColor.b);
		std::cout << "x: " << dvd_position.x << " y: " << dvd_position.y << std::endl;
	}
	else if (dvd_position.y <= y_min) {
		dvd_velocity.y = -dvd_velocity.y;
		dvd_position.y = y_min;
		glUseProgram(shaderProgram);
		glm::vec3 randomColor = getRandomColor();
		glUniform3f(glGetUniformLocation(shaderProgram, "colorB"), randomColor.r, randomColor.g, randomColor.b);
		std::cout << "x: " << dvd_position.x << " y: " << dvd_position.y << std::endl;
	}

	return dvd_position;
}

glm::vec3 getRandomColor()
{
	return glm::vec3(randomColor(), randomColor(), randomColor());
}

float randomColor()
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}
