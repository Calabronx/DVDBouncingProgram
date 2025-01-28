/**
	Programmed by Sebastian Calabro
**/
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <math.h>
#include <windows.h> 

#define TAU (3.14159265358979323846 * 2.0)

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow* window);
void updateColor(float time);
void setRectangleSize(float vertices[]);
void setUniformRandomColor();
void resetInitialBackroundColor();
void resetTextureInitialColor();
glm::vec3 moveDVDSprite(float dt, unsigned int window_width, glm::vec2 size);
glm::vec3 getRandomColor();
float rand(glm::vec2 co);
float randomColor();
namespace {
	// window settings
	const unsigned int SCR_WIDTH = 800;
	const unsigned int SCR_HEIGHT = 600;
	const float window_x_max = 0.917626f;
	const float window_x_min = 0.817626f;
	const float window_y_max = 0.90f;
	const float window_y_min = -0.90f;
	// max delta frames
	float max_dt = 3.29149f;
	float currentFrame = 0.0f;
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;
	float d = TAU;
	//dvd rectangle properties
	float dvd_velocity_float = 0.35f;
	float width = 0.0f; // Ancho como float, resultando en 0.25f
	float height = 0.0f; 
	const float area = width * height;
	glm::vec3 dvd_position;
	glm::vec3 dvd_velocity(0.0004f, -0.0002f, 0.0f);
	glm::vec3 dvd_size = glm::vec3(width,height, 0.0f); // Ancho en el eje X, cero en Y y Z

	glm::vec4 background_color(0.0f, 0.0f, 0.0f, 0.0f);
	//shaders
	unsigned int shaderProgram;
	unsigned int shaderProgram2;
	const char* vertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"layout (location = 1) in vec3 aColor;\n"
		"layout (location = 2) in vec2 aTexCoord;\n"
		"out vec3 dvdColor;\n"
		"out vec2 TexCoord;\n"
		"uniform mat4 transform;"
		"void main()\n"
		"{\n"
		"   gl_Position = transform * vec4(aPos, 1.0);\n"
		"	dvdColor = aColor;\n"
		"   TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
		"}\0";
	const char* fragmentShaderSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec3 dvdColor;\n"
		"in vec2 TexCoord;\n"
		"uniform sampler2D texture;\n"
		"uniform vec3 randomColor;\n"
		"void main()\n"
		"{\n"
		"   FragColor = texture(texture, TexCoord)* vec4(randomColor, 1.0);\n"
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

	bool random_background_color = false;
	bool dvd_changing_color_off = false;
	bool mouse_aim = false;
	bool random_dvd_color = false;
}

int main(void)
{
	// Window settings
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
	glfwSetMouseButtonCallback(window, mouse_button_callback);
		
	int window_width, window_height;
	glfwGetWindowSize(window, &window_width, &window_height);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();


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

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	const char* fragmentShaderNoiseSource = "#version 330 core\n"
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

	unsigned int fragmentShaderNoise = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderNoise, 1, &fragmentShaderNoiseSource, NULL);
	glCompileShader(fragmentShaderNoise);
	glGetShaderiv(fragmentShaderNoise, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	shaderProgram2 = glCreateProgram();
	glAttachShader(shaderProgram2, vertexShader);
	glAttachShader(shaderProgram2, fragmentShaderNoise);
	glLinkProgram(shaderProgram2);

	glGetProgramiv(shaderProgram2, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram2, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteShader(fragmentShaderNoise);

	float vertices[] = {
		// positions              // colors              // texture cords
		 0.125f,  0.125f, -0.25f, 1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
		 0.125f, -0.125f, -0.25f, 0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
		-0.125f, -0.125f, -0.25f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
		-0.125f,  0.125f, -0.25f, 1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left

	};
	
	float v1 = vertices[0];
	float v2 = -v1;
	float _width = (vertices[0] - (-vertices[0]));// Ancho como float, resultando en 0.25f
	float _height = (vertices[1] - (-vertices[0]));

		// 0.375

	glm::vec2 size(_width, _height);

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

	resetTextureInitialColor();

	float last_velocity = 0.35f;
	float scaleX = 1.0f;
	float scaleY = 1.0f;

	bool app_pause = false;
	
	// loop
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(background_color.r, background_color.g, background_color.b, background_color.a);
		glBindVertexArray(VAO);
		glBindTexture(GL_TEXTURE_2D, texture);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// (Your code calls glfwPollEvents())
		// ...
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Propiedades", NULL, ImGuiWindowFlags_None | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		ImGui::SetWindowSize(ImVec2(500, 250));
		//ImGui::ShowDemoWindow(); // Show demo window! :)
		if (ImGui::Button("Color de fondo aleatorio")) {
			random_background_color = !random_background_color;
			if (!random_background_color) {
				resetInitialBackroundColor();
			}
			std::cout << "CLICK" << std::endl;
		}
		if (random_background_color) {
		
			updateColor(glfwGetTime());
		}

		if (ImGui::Button("Noise")) {
		/*	background_color.r = rand(glm::vec2(12.9898, 78.233));
			background_color.g = rand(glm::vec2(12.9898, 78.233));
			background_color.b = rand(glm::vec2(12.9898, 78.233));
			background_color.a = rand(glm::vec2(12.9898, 78.233));*/
			dvd_changing_color_off = true;
			glUseProgram(shaderProgram2);
		}

		if (ImGui::Button("play/pausa")) {
			//dvd_velocity_float = last_velocity;
			app_pause = !app_pause;
		}
		/*if (ImGui::Button("pausa")) {
			last_velocity = dvd_velocity_float;
			dvd_velocity_float = 0.0f;
		}*/

		/*if (app_pause) {
			if (dvd_velocity_float > 0.0f)
				last_velocity = dvd_velocity_float;

			dvd_velocity_float = 0.0f;
		}
		else {
			dvd_velocity_float = last_velocity;
		}*/

		if (ImGui::Button("cambio de color off")) {
			dvd_changing_color_off = !dvd_changing_color_off;
			resetTextureInitialColor();
		}

		if (ImGui::Button("cambio color textura random")) {
			random_dvd_color = !random_dvd_color;
		}

		if (random_dvd_color) {
			setUniformRandomColor();
		}

	/*	if (ImGui::Button("mouse aim")) {
			mouse_aim = !mouse_aim;
		}*/

		if (ImGui::Button("reset")) {
			//{x=0.00104558410 r=0.00104558410 s=0.00104558410 ...}
			dvd_position.x = 0.00104558410;
			dvd_position.y = 0.00104558410;
			//dvd_velocity(0.0004f, -0.0002f, 0.0f);
			dvd_velocity.x = 0.0004f;
			dvd_velocity.y = -0.0002f;
			dvd_velocity_float = 0.35f;
			glUseProgram(shaderProgram);
			glm::vec3 initialColor = glm::vec3(1.0f, 1.0f, 1.0f);
			glUniform3f(glGetUniformLocation(shaderProgram, "randomColor"), initialColor.r, initialColor.g, initialColor.b);
			resetInitialBackroundColor();

		}
	
		ImGui::SliderFloat("Velocidad", &dvd_velocity_float, 0.0f, 3.0f);
		if (ImGui::SliderFloat("Ancho", &scaleX, 0.0f, 3.0f)) {
		
			setRectangleSize(vertices);

		}
		ImGui::SliderFloat("Largo", &scaleY, 0.0f, 3.0f);
		//ImGui::SliderFloat("Size", &dvd_velocity_float, 0.0f, 3.0f);
		ImGui::ColorEdit4("Background color", &background_color[0]);
		ImGui::End();

		// Rendering
		// (Your code clears your framebuffer, renders your other stuff etc.)
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		// (Your code calls glfwSwapBuffers() etc.)

		//transforms
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, moveDVDSprite((float)glfwGetTime(), SCR_WIDTH, size));
		transform = glm::scale(transform, glm::vec3(scaleX, scaleY, 1.0f));
		unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
		/* Swap front and back buffers */
		glfwSwapBuffers(window);
		/* Poll for and process events */
		glfwPollEvents();
	}
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteProgram(shaderProgram);
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		/*dvd_velocity.x = xpos;
		dvd_velocity.y = ypos;*/
		glm::vec2 mouse_pos(xpos, ypos);
		float x = mouse_pos.x - dvd_position.x;
		float y = mouse_pos.y - dvd_position.y;
		glm::vec2 direction (x,y);
		/*dvd_position.x = direction.x;
		dvd_position.y = direction.y;*/
		std::cout << "Cursor Position at (" << xpos << " : " << ypos << std::endl;
	}

}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void updateColor(float time)
{
	float r = (sin(time) + 1.0f) / 2.0f;
	float g = (sin(time + 2.0f) + 1.0f) / 2.0f;
	float b = (sin(time + 4.0f) + 1.0f) / 2.0f;
	float a = (sin(time + 5.0f) + 1.0f) / 2.0f;
	background_color.r = r;
	background_color.g = g;
	background_color.b = b;
	background_color.a = a;
	std::cout << r << " " << g << " " << b << std::endl;
	glClearColor(r, g, b, a);
}

void setRectangleSize(float vertices[])
{
	int size = sizeof(vertices);
	for (int i = 0; i < size; i++) {
		vertices[i] = vertices[i] * 2;
	}
}

glm::vec3 moveDVDSprite(float dt, unsigned int window_width, glm::vec2 size)
{
	currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	d += deltaTime * dvd_velocity_float;
	dvd_position += dvd_velocity * d * dvd_velocity_float;

	float half_width = size.x / 2.0f;
	float half_height = size.y / 2.0f;

	if (d >= max_dt)
		d = max_dt;
	
	if (dvd_position.x - half_width + 0.10f <= -window_x_max) {
		dvd_velocity.x = -dvd_velocity.x;
		dvd_position.x = -window_x_max + half_width - 0.10f; // Ajuste adicional
		setUniformRandomColor();
	}
	else if (dvd_position.x + half_width - 0.10f >= window_x_max) {
		dvd_velocity.x = -dvd_velocity.x;
		dvd_position.x = window_x_max - half_width + 0.10f; // Ajuste adicional
		setUniformRandomColor();
	}

	if (dvd_position.y + half_height - 0.10f >= window_y_max) {
		dvd_velocity.y = -dvd_velocity.y;
		dvd_position.y = window_y_max - half_height + 0.10f; // Ajuste adicional
		setUniformRandomColor();
	}
	else if (dvd_position.y - half_height + 0.10f <= window_y_min) {
		dvd_velocity.y = -dvd_velocity.y;
		dvd_position.y = window_y_min + half_height - 0.10f; // Ajuste adicional
		setUniformRandomColor();
	}
	//std::cout << "area " << area <<  std::endl;

	//std::cout << "x: " << dvd_position.x << " y: " << dvd_position.y << std::endl;
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

void setUniformRandomColor() {
	if (dvd_changing_color_off) 
		return;

	if (random_dvd_color) {
		glUseProgram(shaderProgram);
		glm::vec3 randomColor;
		float time = glfwGetTime();
		float r = (sin(time) + 1.0f) / 2.0f;
		float g = (sin(time + 2.0f) + 1.0f) / 2.0f;
		float b = (sin(time + 4.0f) + 1.0f) / 2.0f;
		float a = (sin(time + 5.0f) + 1.0f) / 2.0f;
		randomColor.r = r;
		randomColor.g = g;
		randomColor.b = b;
		glUniform3f(glGetUniformLocation(shaderProgram, "randomColor"), randomColor.r, randomColor.g, randomColor.b);
		return;
	}
	
	glUseProgram(shaderProgram);
	glm::vec3 randomColor = getRandomColor();
	glUniform3f(glGetUniformLocation(shaderProgram, "randomColor"), randomColor.r, randomColor.g, randomColor.b);
}

void resetInitialBackroundColor()
{
	background_color.r = 0.0f;
	background_color.g = 0.0f;
	background_color.b = 0.0f;
	background_color.a = 0.0f;
}

void resetTextureInitialColor()
{
	glUseProgram(shaderProgram);
	glm::vec3 initialColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(shaderProgram, "randomColor"), initialColor.r, initialColor.g, initialColor.b);
}

float rand(glm::vec2 co) {
	return glm::fract(sin(glm::dot(co.x, co.y)) * 43758.5453);

}