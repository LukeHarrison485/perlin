#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdint.h>

typedef struct {
    float x;
    float y;
} vector2; // 2D vector.

// Creates a psuedo-Random value using a seed values, this is done so that the result of the alorithim can be replicated in perlin noise generation
vector2 randomGradient(int ix, int iy, int seed) {
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2; 
    unsigned a = ix + seed, b = iy + seed;
    // Peforms a large multication operation using a large prime number making the number look more random
    a *= 3284157443;
    // Creates a value using a and the size of w + w / 2 making B appear more random
    b ^= a << s | a >> (w - s);
    b *= 1911520717;
    // Merges a and b seamlessly
    a ^= b << s | b >> (w - s);
    a *= 2048419325;

    // Weighs the result of a to from 0 to 2 radiants
    float random = a * (3.14159265 / ~(~0u >> 1)); // [0, 2*Pi]
    //Creates a vector value ranging from -1 to 1
    vector2 v = { sin(random), cos(random) };
    return v;
}

float dotProduct(int ix, int iy, float x, float y, int seed) {
    // Creates a random Gradient for usage in creating a random value for producing a dot product value between the corners and interception point
    vector2 grad = randomGradient(ix, iy, seed);

    // Obtains the fractionial parts of x and y
    float dx = x - (float)ix;
    float dy = y - (float)iy;
    return dx * grad.x + dy * grad.y;
}

float smoothStep(float a, float b, float w) {
    // Smooths the results of each corner to produce a smoother looking perlin noise result using a weight.
    return (b - a) * (3.0 - w * 2.0) * w * w + a;
}

float perlinNoise(float x, float y, int seed) {
    int x0 = (int)x;
    int y0 = (int)y;
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    float xf = x - (float)x0;
    float yf = y - (float)y0;

    // Creates the values of the bottom left and bottom right corners using the dot product function to make the noise have a gradient effect
    float c0 = dotProduct(x0, y0, x, y, seed);
    float c1 = dotProduct(x1, y0, x, y, seed);
    // Makes the noise look "Smoother"
    float val1 = smoothStep(c0, c1, xf);

    //Top left and Top right this time
    float c2 = dotProduct(x0, y1, x, y, seed);
    float c3 = dotProduct(x1, y1, x, y, seed);
    float val2 = smoothStep(c2, c3, xf);

    // Returns the noise value using the given coordinates
    return smoothStep(val1, val2, yf);
}

float fractalNoise(float scale, int octaves, float pers, float lac, int x, int y, int seed) {
    float val = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float maxHeight = 0.0f;
    for(int i = 0; i < octaves; i++) {
        float noiseVal = perlinNoise((x / scale) * frequency, (y / scale) * frequency, seed);
        val += noiseVal * amplitude;
        maxHeight += amplitude;
        amplitude *= pers;
        frequency *= lac;
    }
    return val;
}