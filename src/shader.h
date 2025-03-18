#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

void setFloat(unsigned int shaderProgram, const char* location, float val) {
    int uniformLocation = glGetUniformLocation(shaderProgram, location);
	glUniform1f(uniformLocation, val);
}

void setVec3(unsigned int shaderProgram, const char* location, vec3 val) {
    int uniformLocation = glGetUniformLocation(shaderProgram, location);
	glUniform3f(uniformLocation, val[0], val[1], val[2]);
}

char* readShaderSource(const char* path) {
	FILE* shaderFile = fopen(path, "rb");
	if(!shaderFile) {
		fprintf(stderr, "Failed to open file at location: %s\n", path);
		return NULL;
	}
	char* content;
	fseek(shaderFile, 0l, SEEK_END);
	long size = ftell(shaderFile);
	fseek(shaderFile, 0l, SEEK_SET); 
	content = (char*)malloc((sizeof(char*) * size) + 1);
	fread(content, 1, size, shaderFile);
	content[size] = '\0';
	fclose(shaderFile);
	return content;
}

unsigned int createShader(const char* pathVs, const char* pathFs) {
	unsigned int vsShader, fsShader;
	vsShader = glCreateShader(GL_VERTEX_SHADER);
	fsShader = glCreateShader(GL_FRAGMENT_SHADER);
	char* vsSource = readShaderSource(pathVs);
	char* fsSource = readShaderSource(pathFs);
	if(!vsSource || !fsSource) {
		fprintf(stderr, "Failed to open shader source!");
	}
	glShaderSource(vsShader, 1, (const char* const*)&vsSource, NULL);
	glShaderSource(fsShader, 1, (const char* const*)&fsSource, NULL);
	free(vsSource);
	free(fsSource);
	glCompileShader(vsShader);
	GLint success;
	glGetShaderiv(vsShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLint length;
		glGetShaderiv(vsShader, GL_INFO_LOG_LENGTH, &length);
		GLchar* log = (GLchar*)malloc(sizeof(GLchar) * length);
		glGetShaderInfoLog(vsShader, length, &length, log);
		fprintf(stderr, "Shader failed to compile! Vertex Shader error:\n%s", log);
		glDeleteShader(vsShader);
		free(log);
		return 0;
	}
	glCompileShader(fsShader);
	glGetShaderiv(fsShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLint length;
		glGetShaderiv(fsShader, GL_INFO_LOG_LENGTH, &length);
		GLchar* log = (GLchar*)malloc(sizeof(GLchar) * length);
		glGetShaderInfoLog(fsShader, length, &length, log);
		fprintf(stderr, "Shader failed to compile! Fragment Shader error:\n%s", log);
		glDeleteShader(fsShader);
		free(log);
		return 0;
	}
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vsShader);
	glAttachShader(shaderProgram, fsShader);

	glLinkProgram(shaderProgram);
	
	GLint isLinked;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, (int *)&isLinked);
	if(!isLinked) {
		GLint length;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &length);
		GLchar* log = (GLchar*)malloc(sizeof(GLchar) * length);
		glGetProgramInfoLog(shaderProgram, length, &length, log);
		fprintf(stderr, "Failed to link shader program! Error:\n%s", log);
		glDeleteProgram(shaderProgram);
		free(log);
		return 0;
	}
	glDetachShader(shaderProgram, vsShader);
	glDetachShader(shaderProgram, fsShader);
	glDeleteShader(vsShader);
	glDeleteShader(fsShader);
	return shaderProgram;
}