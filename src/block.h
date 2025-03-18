#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <cglm/cglm.h>
#include <string.h>
#include "perlin.h"
#include <pthread.h>

#define CHUNK_SIZE 32

typedef struct {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
} Vertex;

typedef struct {
    uint16_t blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    Vertex* vertices;
    int vertexCount;
    vec3 pos;
    GLuint VBO, VAO;
} Chunk;
Chunk *chunks;

typedef struct {
    Chunk** chunks;
    int size;
    int front;
    int capacity;
    pthread_mutex_t mutex;
} Queue;

Queue createQueue(int capacity) {
    Queue newQueue;
    newQueue.capacity = capacity;
    newQueue.chunks = malloc(sizeof(chunks) * capacity);
    newQueue.front = 0;
    newQueue.size = 0;
    pthread_mutex_init(&newQueue.mutex, NULL);
}

void enqueue(Queue* queue, Chunk* chunk) {
    pthread_mutex_lock(&queue->mutex);
    if(queue->size == queue->capacity) {
        return;
    }
    queue->chunks[(queue->front + queue->size) % queue->capacity] = chunk;
    queue->size++;
    pthread_mutex_unlock(queue->mutex);
}

Chunk* dequeue(Queue* queue) {
    pthread_mutex_lock(&queue->mutex);
    if(queue->size == 0) {
        return NULL;
    }
    Chunk* chunk = queue->chunks[queue->front];
    queue->front = (queue->front + queue->size) % queue->capacity;
    queue->size--;
    pthread_mutex_unlock(queue->mutex);
    return chunk;
}

typedef enum {FRONT, BACK, LEFT, RIGHT, TOP, BOTTOM} Face;

int MAX_VERTICES = 36 * (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);

static const float faceVertices[6][4][3] = {
	{ {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1} },  // FRONT
	{ {1, 0, 0}, {0, 0, 0}, {0, 1, 0}, {1, 1, 0} },  // BACK
	{ {0, 0, 0}, {0, 0, 1}, {0, 1, 1}, {0, 1, 0} },  // LEFT
	{ {1, 0, 1}, {1, 0, 0}, {1, 1, 0}, {1, 1, 1} },  // RIGHT
	{ {0, 1, 1}, {1, 1, 1}, {1, 1, 0}, {0, 1, 0} },  // TOP
	{ {0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1} }   // BOTTOM
};

static const float normals[6][3] = {
    {  0,  0,  1 }, // FRONT
    {  0,  0, -1 }, // BACK
    { -1,  0,  0 }, // LEFT
    {  1,  0,  0 }, // RIGHT
    {  0,  1,  0 }, // TOP
    {  0, -1,  0 }  // BOTTOM
};

static const float texCoords[6][2] = {
    {0, 0}, {1, 0}, {1, 1},  // First triangle
    {1, 1}, {0, 1}, {0, 0}   // Second triangle
};

bool isBlockVisible(int x, int y, int z, Chunk* chunk) {
    if (x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE || y >= CHUNK_SIZE || z >= CHUNK_SIZE) {
        return false;
    }
    return chunk->blocks[x][y][z] == 0; //If this is an air block we can assume that the block face of it's neighboring block is visible during mesh creation
}

void addFace (Chunk* chunk, float x, float y, float z, Face face) {
    int indices[6] = { 0, 1, 2, 2, 3, 0 };
    for(int i = 0; i < 6; i++) {
        Vertex vertex;
        int idx = indices[i];
        vertex.x = x + faceVertices[face][idx][0];
        vertex.y = y + faceVertices[face][idx][1];
        vertex.z = z + faceVertices[face][idx][2];
        vertex.nx = normals[face][0];
        vertex.ny = normals[face][1];
        vertex.nz = normals[face][2];
        vertex.u = texCoords[i][0];
        vertex.v = texCoords[i][1];
        chunk->vertices[chunk->vertexCount] = vertex;
        chunk->vertexCount++;
    }
    
}

void createChunkMesh(Chunk* chunk) {
    chunk->vertexCount = 0;
    chunk->vertices = (Vertex*)malloc(sizeof(Vertex) * MAX_VERTICES);
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                if (chunk->blocks[x][y][z] != 0) {
                    if (isBlockVisible(x, y, z + 1, chunk)) addFace(chunk, x, y, z, FRONT);
                    if (isBlockVisible(x, y, z - 1, chunk)) addFace(chunk, x, y, z, BACK);
                    if (isBlockVisible(x - 1, y, z, chunk)) addFace(chunk, x, y, z, LEFT);
                    if (isBlockVisible(x + 1, y, z, chunk)) addFace(chunk, x, y, z, RIGHT);
                    if (isBlockVisible(x, y + 1, z, chunk)) addFace(chunk, x, y, z, TOP);
                    if (isBlockVisible(x, y - 1, z, chunk)) addFace(chunk, x, y, z, BOTTOM);
                }
            }
        }
    }
    chunk->vertices = (Vertex*)realloc(chunk->vertices, sizeof(Vertex) * chunk->vertexCount);
}

void createChunkData(Chunk* chunk, int seed) {
    float scale = 64.0f;
    for(int x = 0; x < CHUNK_SIZE; x++) {
        for(int z = 0; z < CHUNK_SIZE; z++) {
            float worldX = chunk->pos[0] + x;
            float worldZ = chunk->pos[2] + z;
            float val = perlinNoise(worldX / scale, worldZ / scale, seed);
            val = (val + 1.0f) * 0.5f * CHUNK_SIZE;
            for(int y = 0; y < CHUNK_SIZE; y++) {
                if(y < val) {
                    chunk->blocks[x][y][z] = 1;
                } 
                else {
                    chunk->blocks[x][y][z] = 0;
                }
            }
        }
    }
}


void uploadChunkToGPU(Chunk* chunk) {
    glGenVertexArrays(1, &chunk->VAO);
    glGenBuffers(1, &chunk->VBO);

    glBindVertexArray(chunk->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO);
    glBufferData(GL_ARRAY_BUFFER, chunk->vertexCount * sizeof(Vertex), chunk->vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);
}

void renderChunk(Chunk* chunk) {
    glBindVertexArray(chunk->VAO);
    glDrawArrays(GL_TRIANGLES, 0, chunk->vertexCount);
    glBindVertexArray(0);
}

void* terrainGenerationThread(void* vargp) {
    
}