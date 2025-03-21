#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <cglm/cglm.h>
#include "block.h"
#include "camera.h"
#include "lighting.h"
#include "texture.h"

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void generateTerrain(int seed);
void removeChunks(Chunk* chunks);
void renderChunks(mat4 model, unsigned int shader);

void configureLighting(unsigned int shader);
void configureMatrices(mat4 view, mat4 model, mat4 projection, unsigned int shader);
void renderScene(unsigned int shader);

void setMat4(unsigned int shaderProgram, const char* location, mat4 value);

float lastTime = 0;
char title[256];

Cam cam;
int windowedWidth = 1280; 
int windowedHeight = 720;
float deltaTime = 0.0f;	
float lastFrame = 0.0f; 
int renderDistance = 20;
int wireFrame;

int main(void) {
	//Init GLfW and create the window
	glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 16);
	
	GLFWwindow* window = glfwCreateWindow(windowedWidth, windowedHeight, "Computer science NEA coursework project", NULL, NULL);
	if(!window) {
		fprintf(stderr, "Failed to create the window of the application!");
	}

	cam = createCamera((vec3){(32 * renderDistance) / 2, 64, (32 * renderDistance) / 2}, 20.0f, 90.0f, 0.5f);

	lastX = (float)windowedWidth / 2.0f;
	lastY = (float)windowedHeight / 2.0f;

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("Failed to load GLAD");
	}

	unsigned int basicShader = createShader("shader/basic.vs", "shader/basic.fs");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_MULTISAMPLE);

	chunks = (Chunk*)malloc(sizeof(Chunk) * (renderDistance * renderDistance));
	if(!chunks) {
		fprintf(stderr, "Failed to alloctate chunk memory, Quiting!\n");
		return -1;
	}
	

	generateTerrain(time(NULL));

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
	textures[DIRT] = loadTexture("dirt.png");
	glUniform1i(glGetUniformLocation(basicShader, "material.diffuse"), textures[DIRT]);

	printf("%s", readShaderSource("shader/basic.vs"));

	while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processCameraInput(window, &cam, deltaTime);

        renderScene(basicShader);
		
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
	glfwTerminate();
	free(chunks);
}
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;
	xoffset *= cam.sensitivity;
	yoffset *= cam.sensitivity;
	cam.yaw += xoffset;
	cam.pitch += yoffset;
	if (cam.pitch > 89.0f) {cam.pitch = 89.0f;}
    if (cam.pitch < -89.0f) {cam.pitch = -89.0f;}
	vec3 direction;
	direction[0] = cos(glm_rad(cam.yaw)) * cos(glm_rad(cam.pitch));
	direction[1] = sin(glm_rad(cam.pitch));
	direction[2] = sin(glm_rad(cam.yaw)) * cos(glm_rad(cam.pitch));
	glm_normalize(direction);
	glm_vec3_copy(direction, cam.cameraFront);
}

void setMat4(unsigned int shaderProgram, const char* location, mat4 value) {
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, location), 1, GL_FALSE, (float*)value);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_G && action == GLFW_RELEASE) {
		if(wireFrame) {
			wireFrame = 0;
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		else {
			wireFrame = 1;
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
	}
	if(key == GLFW_KEY_R && action == GLFW_RELEASE) {
		
		generateTerrain(time(NULL));
	}
}

void generateTerrain(int seed) {
	float timeBefore = glfwGetTime();
	for (int x = 0; x < renderDistance; x++) {
		for (int z = 0; z < renderDistance; z++) {
			Chunk newChunk; 
			glm_vec3_copy((vec3){x * CHUNK_SIZE, 0, z * CHUNK_SIZE}, newChunk.pos);
			createChunkData(&newChunk, seed);
			createChunkMesh(&newChunk);
			uploadChunkToGPU(&newChunk);
			chunks[x * renderDistance + z] = newChunk;
		}
	}
	float timeAfter = glfwGetTime();
	printf("\nTime taken: %f\n", timeAfter - timeBefore);
}

void removeChunks(Chunk* chunks) {
	free(chunks);
	chunks = (Chunk*)malloc(sizeof(Chunk) * (renderDistance * renderDistance));
}

void renderChunks(mat4 model, unsigned int shader) {
	for(int x = 0; x < renderDistance; x++) {
		for(int z = 0; z < renderDistance; z++) {
			glm_mat4_identity(model); // Reset model matrix
			glm_translate(model, chunks[x * renderDistance + z].pos); // Apply chunk1's position
			setMat4(shader, "model", model);
			renderChunk(&chunks[x * renderDistance + z]);
		}
	}
}

void configureLighting(unsigned int shader) {
	glUniform3fv(glGetUniformLocation(shader, "viewPos"), 1, cam.cameraPos);
	createDirLight(shader, (vec3){-0.2f, -1.0f, -0.3f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.4f, 0.4f, 0.4f}, (vec3){0.4f, 0.4f, 0.4f});
	glUniform1f(glGetUniformLocation(shader, "material.shininess"), 64.0f);
	glUniform1i(glGetUniformLocation(shader, "numPointLights"), numOfPointLights);
	pointLightShaderPassthrough(shader);
}

void configureMatrices(mat4 view, mat4 model, mat4 projection, unsigned int shader) {
	glm_mat4_identity(model);
	glm_perspective(glm_rad(cam.fov), (float)windowedWidth / (float)windowedHeight, 0.1f, 100000.0f, projection);
	updateView(cam, view);
	setMat4(shader, "projection", projection);
	setMat4(shader, "view", view);
}

void renderScene(unsigned int shader) {
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shader);

	configureLighting(shader);

	mat4 model, projection, view;
	configureMatrices(view, model, projection, shader);

	renderChunks(model, shader);
}