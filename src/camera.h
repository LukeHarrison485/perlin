#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <cglm/cglm.h>

float lastX;
float lastY;

typedef struct {
    vec3 cameraPos;
    vec3 cameraFront;
    vec3 cameraUp;
    float speed;
    float fov;
    float sensitivity;
    float pitch;
    float yaw;
    mat4 view;
} Cam;

Cam createCamera(vec3 pos, float speed, float fov, float sens) {
    Cam cam;
    glm_vec3_copy(pos, cam.cameraPos);
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, cam.cameraUp);
    glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, cam.cameraFront);
    cam.fov = fov;
    cam.speed = speed;
    cam.sensitivity = sens;
    cam.yaw = -90.0f;
    glm_mat4_identity(cam.view);
    return cam;
}

void updateView(Cam cam, mat4 view) {
    vec3 target;
    glm_vec3_add(cam.cameraPos, cam.cameraFront, target);
    glm_lookat(cam.cameraPos, target, cam.cameraUp, view);
}

void processCameraInput(GLFWwindow* window, Cam* cam, float deltaTime) {
    vec3 temp;
    float velocity = cam->speed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        glm_vec3_scale(cam->cameraFront, velocity, temp);
        glm_vec3_add(cam->cameraPos, temp, cam->cameraPos);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        glm_vec3_scale(cam->cameraFront, velocity, temp);
        glm_vec3_sub(cam->cameraPos, temp, cam->cameraPos);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        glm_cross(cam->cameraFront, cam->cameraUp, temp);
        glm_normalize(temp);
        glm_vec3_scale(temp, velocity, temp);
        glm_vec3_sub(cam->cameraPos, temp, cam->cameraPos);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        glm_cross(cam->cameraFront, cam->cameraUp, temp);
        glm_normalize(temp);
        glm_vec3_scale(temp, velocity, temp);
        glm_vec3_add(cam->cameraPos, temp, cam->cameraPos);
    }
}

