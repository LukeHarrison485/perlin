#include <cglm/cglm.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"

typedef struct 
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float constant;
	float linear;
	float quadratic;
	vec3 position;
} pointLight;

pointLight pointLightArr[100];
int numOfPointLights = 0;

void createPointLight(vec3 pos, vec3 amb, vec3 diff, vec3 spec, float constant, float lin, float quad) {
	glm_vec3_copy(pos, pointLightArr[numOfPointLights].position);
	glm_vec3_copy(amb, pointLightArr[numOfPointLights].ambient);
	glm_vec3_copy(diff, pointLightArr[numOfPointLights].diffuse);
	glm_vec3_copy(spec, pointLightArr[numOfPointLights].specular);
	pointLightArr[numOfPointLights].constant = constant;
	pointLightArr[numOfPointLights].linear = lin;
	pointLightArr[numOfPointLights].quadratic = quad;
	numOfPointLights++;
}

void createDirLight(unsigned int shaderProgram, vec3 dir, vec3 ambient, vec3 spec, vec3 diff) {
	glUniform3fv(glGetUniformLocation(shaderProgram, "dirLight.direction"), 1, dir);
	glUniform3fv(glGetUniformLocation(shaderProgram, "dirLight.ambient"), 1, ambient);
	glUniform3fv(glGetUniformLocation(shaderProgram, "dirLight.specular"), 1, spec);
	glUniform3fv(glGetUniformLocation(shaderProgram, "dirLight.diffuse"), 1, diff);
}

void pointLightShaderPassthrough(unsigned int shaderProgram) {
	char baseName[50];
	char fullName[100];
	for(int i = 0; i < numOfPointLights; i++) {
		snprintf(baseName, sizeof(baseName), "pointLights[%d]", i);

		snprintf(fullName, sizeof(fullName), "%s.position", baseName);
		setVec3(shaderProgram, fullName, pointLightArr[i].position);

		snprintf(fullName, sizeof(fullName), "%s.ambient", baseName);
		setVec3(shaderProgram, fullName, pointLightArr[i].ambient);

		snprintf(fullName, sizeof(fullName), "%s.diffuse", baseName);
		setVec3(shaderProgram, fullName, pointLightArr[i].diffuse);

		snprintf(fullName, sizeof(fullName), "%s.specular", baseName);
		setVec3(shaderProgram, fullName, pointLightArr[i].specular);

		snprintf(fullName, sizeof(fullName), "%s.constant", baseName);
		setFloat(shaderProgram, fullName, pointLightArr[i].constant);

		snprintf(fullName, sizeof(fullName), "%s.linear", baseName);
		setFloat(shaderProgram, fullName, pointLightArr[i].linear);

		snprintf(fullName, sizeof(fullName), "%s.quadratic", baseName);
		setFloat(shaderProgram, fullName, pointLightArr[i].quadratic);
	}
}