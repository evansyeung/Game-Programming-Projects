#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include "ShaderProgram.h"
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
#define LEVEL_HEIGHT 25
#define LEVEL_WIDTH 130
#define SPRITE_COUNT_X 8
#define SPRITE_COUNT_Y 3

//Define LEVEL_HEIGHT and WIDTH

float TILE_SIZE = 0.5f;
int mapWidth;
int mapHeight;

const int runAnimation[] = {9, 10, 11, 12, 13};
const int numFrames = 5;
float animationElapsed = 0.0f;
float framesPerSecond = 30.0f;
int currentIndex = 0;

const int runAnimationRuby[] = {0, 1, 2, 3, 4, 5, 6};
const int numFramesRuby = 7;
float animationElapsedRuby = 0.0f;
float framesPerSecondRuby = 30.0f;
int currentIndexRuby = 0;

float friction = 2.0f;
float gravity = 2.0f;
float penetration = 0.0f;
float y_distance, x_distance;
float adjustY, adjustX;
float accelerationX = 1.0f, accelerationY;
bool onFloor = false, moveJump = false, moveDown = false, moveLeft = false, moveRight = false;
int score = 0;

//unsigned char** levelData;


enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL1, STATE_GAME_LEVEL2, STATE_GAME_LEVEL3, STATE_GAME_END };
int state = STATE_MAIN_MENU;

float lastFrameTicks = 0.0f;
const Uint8 *keys = SDL_GetKeyboardState(NULL);


unsigned char levelData[LEVEL_HEIGHT][LEVEL_WIDTH] =
{
    {12,12,12,12,12,12,12,12,12,12,12,12,12,12,16,12,12,16,12,12,14,12,12,12,12,16,12,12,12,12,12,12,12,16,12,12,12,12,12,16,12,12,12,12,12,12,12,14,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,14,12,12,12,14,12,12,15,12,12,12,12,15,12,12,12,12,12,12,12,12,12,12,12,12,12,15,12,12,12,12,12,12,12,22,15,5,22,22,12,16,12,16,12,12,12,15,23,23,23,15,12,12},
    {12,15,14,12,12,12,12,12,12,12,14,12,14,12,12,12,12,12,12,12,12,12,12,12,14,12,12,12,15,12,15,12,12,12,12,12,14,12,12,12,12,12,16,12,12,12,12,12,12,12,12,12,12,16,12,12,12,12,12,12,16,12,12,12,12,12,12,12,12,14,12,12,12,16,12,12,12,12,12,12,12,12,12,12,12,12,14,12,12,12,12,16,12,16,12,15,12,12,12,15,12,12,12,12,12,12,14,12,14,12,1,2,2,7,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {14,12,12,15,22,22,22,15,12,12,12,12,12,12,12,12,12,16,12,12,12,12,12,12,12,12,12,12,12,12,12,12,14,12,12,12,12,12,12,14,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,14,12,12,12,12,12,12,12,12,12,14,12,15,12,12,12,16,12,12,12,12,12,12,12,12,12,12,12,12,14,12,12,14,12,12,12,12,12,12,12,12,12,12,15,12,12,14,12,12,12,12,12,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {12,12,1,2,2,2,2,2,3,12,12,12,12,12,12,12,12,12,12,12,16,12,12,12,12,12,16,12,12,15,12,12,12,12,12,12,12,12,12,12,12,12,12,12,16,12,12,12,12,12,15,15,12,12,12,12,12,12,12,12,12,12,12,16,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,14,12,12,14,12,12,12,12,12,12,12,16,12,16,12,12,15,12,12,12,12,12,12,12,12,14,12,12,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {12,12,9,10,10,10,10,10,11,12,16,12,12,12,12,14,15,12,12,12,12,12,12,14,12,12,12,12,12,12,12,12,12,12,15,12,12,15,12,12,12,12,12,14,12,12,12,12,15,15,12,12,12,12,12,12,16,12,12,12,14,22,12,12,12,12,12,12,12,12,12,12,12,16,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,22,22,22,12,23,12,12,12,12,14,12,12,12,12,12,4,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {12,16,9,10,10,10,10,10,11,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,14,12,12,12,12,12,12,12,12,12,12,12,23,12,12,12,12,12,12,12,12,12,12,12,12,12,15,12,12,12,12,12,12,5,12,12,12,12,12,12,15,12,12,12,12,16,12,12,14,12,14,12,12,12,12,0,12,15,12,23,22,12,12,1,2,2,2,2,2,3,12,12,16,12,12,12,12,12,15,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {16,12,9,10,10,10,10,10,11,12,12,12,12,16,12,12,12,12,12,12,12,15,12,12,16,12,4,12,12,12,12,12,15,12,12,12,12,12,12,15,5,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,13,14,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,14,12,16,12,12,23,12,1,2,3,12,17,18,18,18,18,18,19,12,12,12,12,12,15,12,12,22,22,22,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {12,12,9,10,10,10,10,10,11,12,12,12,12,12,12,12,16,12,12,12,12,12,12,12,12,12,12,12,12,12,16,12,12,12,16,12,14,12,23,12,13,22,22,12,12,14,12,12,12,12,12,12,12,12,12,12,12,12,23,22,22,13,12,22,12,12,12,12,12,12,12,22,12,12,12,12,12,12,12,12,12,12,12,23,12,5,12,17,18,19,12,12,24,12,12,24,24,24,14,12,12,12,12,12,12,1,2,2,2,2,8,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {22,22,9,10,10,10,10,10,11,12,12,23,12,12,12,12,12,12,12,12,12,12,12,12,12,12,16,12,12,12,12,12,12,12,12,12,12,12,1,2,7,2,2,3,12,12,12,12,12,12,12,23,12,12,12,12,12,12,1,2,2,7,2,3,12,12,12,16,12,23,1,3,12,12,22,12,12,12,12,23,22,12,1,3,12,21,12,24,14,24,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {2,2,2,2,2,2,2,2,2,2,2,3,12,12,12,12,12,12,12,14,15,12,12,12,12,22,22,22,12,22,22,22,12,12,12,12,12,12,9,10,10,10,10,11,12,12,12,14,12,12,12,1,2,3,12,12,12,12,9,10,10,10,10,11,12,12,12,12,23,1,8,11,15,12,1,2,2,2,2,2,3,12,17,19,12,24,12,12,12,14,12,15,12,16,12,14,12,12,12,12,12,12,15,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,11,12,15,12,12,12,16,12,12,12,12,12,23,12,1,2,3,12,1,2,3,12,16,12,15,12,12,9,10,10,10,10,11,12,12,12,12,12,12,12,17,18,19,12,12,12,12,9,10,10,10,10,11,12,12,12,23,1,8,10,11,12,12,9,10,10,10,10,10,11,12,24,22,22,22,12,12,22,22,22,23,22,12,12,12,12,16,12,23,22,22,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,11,12,12,12,12,12,12,12,16,12,12,12,5,12,17,18,19,12,17,18,19,12,12,12,12,12,12,9,10,10,10,10,11,12,12,12,12,12,15,12,12,12,24,12,12,22,22,9,10,10,10,1,2,2,2,2,2,8,10,10,11,12,12,9,10,10,10,10,10,11,12,12,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,12,12,16,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,11,12,12,12,14,12,12,12,12,12,12,5,13,12,24,24,15,12,24,24,24,12,12,12,12,12,12,9,10,10,10,10,11,12,12,12,12,12,12,23,22,12,12,12,1,2,2,2,2,2,2,8,10,10,10,10,10,10,10,10,11,15,12,9,10,10,10,10,10,11,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,22,23,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,11,22,22,12,12,12,12,12,12,23,5,13,13,23,12,12,12,12,12,12,12,12,22,23,12,12,12,9,10,10,10,10,11,22,12,12,12,12,23,1,2,2,2,2,8,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,9,10,10,10,10,10,11,12,12,17,18,18,18,10,10,10,10,10,10,10,10,10,10,10,18,18,18,19,12,1,2,3,20,20,20,20,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,7,7,7,3,16,12,12,12,12,12,12,12,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,8,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,9,10,10,10,10,10,11,12,12,24,24,12,12,9,10,10,10,10,10,10,10,10,10,11,12,24,24,24,12,9,10,11,20,20,20,20,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,15,12,12,12,15,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,15,9,10,10,10,10,10,11,15,12,12,14,12,14,9,10,10,10,10,10,10,10,10,10,11,14,12,12,12,12,9,10,11,20,20,20,20,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,15,12,12,12,12,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,9,10,10,10,10,10,11,12,12,14,12,12,12,9,10,10,10,10,10,10,10,10,10,11,12,14,12,12,12,9,10,11,20,20,20,20,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,16,12,12,14,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,9,10,10,10,10,10,11,12,12,14,12,14,12,9,10,10,10,10,10,10,10,10,10,11,12,14,12,15,12,9,10,11,20,20,20,20,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,12,12,12,15,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,15,12,9,10,10,10,10,10,11,15,12,12,12,12,14,9,10,10,10,10,10,10,10,10,10,11,12,12,12,12,12,9,10,11,20,20,20,20,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,16,12,12,12,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,9,10,10,10,10,10,11,12,14,12,12,12,12,9,10,10,10,10,10,10,10,10,10,11,12,12,12,12,15,9,10,11,20,20,20,20,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,16,12,12,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,9,10,10,10,10,10,11,12,12,12,12,15,12,9,10,10,10,10,10,10,10,10,10,11,12,15,12,12,12,9,10,11,20,20,20,20,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,12,12,12,12,15,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,9,10,10,10,10,10,11,12,12,12,12,12,12,9,10,10,10,10,10,10,10,10,10,11,12,12,12,12,12,9,10,11,20,20,20,20,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,12,12,12,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,9,10,10,10,10,10,11,12,12,15,12,12,12,9,10,10,10,10,10,10,10,10,10,11,12,12,15,12,12,9,10,11,20,20,20,20,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,14,12,12,15,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,9,10,10,10,10,10,11,12,12,12,12,12,12,9,10,10,10,10,10,10,10,10,10,11,12,12,12,12,15,9,10,11,20,20,20,20,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,15,12,12,12,12,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,9,10,10,10,10,10,11,12,12,12,12,14,12,9,10,10,10,10,10,10,10,10,10,11,12,12,12,12,12,9,10,11,20,20,20,20,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10}
};

//-----------------------------------------------------------------------------------------

GLuint LoadTexture(const char *image_path) {
    SDL_Surface *surface = IMG_Load(image_path);
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_BGRA,
                 GL_UNSIGNED_BYTE, surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    SDL_FreeSurface(surface);
    return textureID;
}

//-----------------------------------------------------------------------------------------

//Draw evenly spaced spritesheets
void DrawSpriteSheetSprite(ShaderProgram *program, int spriteTexture, float vertices[] ,int index, int spriteCountX, int spriteCountY) {
    float u = (float)(((int)index) % spriteCountX) / (float) spriteCountX;
    float v = (float)(((int)index) / spriteCountX) / (float) spriteCountY;
    float spriteWidth = 1.0/(float)spriteCountX;
    float spriteHeight = 1.0/(float)spriteCountY;
    GLfloat texCoords[] = {
        u, v+spriteHeight,
        u+spriteWidth, v,
        u, v,
        u+spriteWidth, v,
        u, v+spriteHeight,
        u+spriteWidth, v+spriteHeight
    };
    // our regular sprite drawing
    
    glBindTexture(GL_TEXTURE_2D, spriteTexture);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

//-----------------------------------------------------------------------------------------

void DrawWorld(ShaderProgram *program, int spriteSheet) {
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    for(int y=0; y < LEVEL_HEIGHT; y++) {
        for(int x=0; x < LEVEL_WIDTH; x++) {
            if(levelData[y][x] != 11) {
                float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float) SPRITE_COUNT_X;
                float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float) SPRITE_COUNT_Y;
                float spriteWidth = 1.0f/(float)SPRITE_COUNT_X;
                float spriteHeight = 1.0f/(float)SPRITE_COUNT_Y;
                vertexData.insert(vertexData.end(), {
                    TILE_SIZE * x, -TILE_SIZE * y,
                    TILE_SIZE * x, (-TILE_SIZE * y)-TILE_SIZE,
                    (TILE_SIZE * x)+TILE_SIZE, (-TILE_SIZE * y)-TILE_SIZE,
                    TILE_SIZE * x, -TILE_SIZE * y,
                    (TILE_SIZE * x)+TILE_SIZE, (-TILE_SIZE * y)-TILE_SIZE,
                    (TILE_SIZE * x)+TILE_SIZE, -TILE_SIZE * y
            });
        
            texCoordData.insert(texCoordData.end(), {
                u, v,
                u, v+(spriteHeight),
                u+spriteWidth, v+(spriteHeight),
                u, v,
                u+spriteWidth, v+(spriteHeight),
                u+spriteWidth, v
                });
           }
        }
    }

    glUseProgram(program->programID);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, spriteSheet);
    glDrawArrays(GL_TRIANGLES, 0, vertexData.size()/2);
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
    
}

//-----------------------------------------------------------------------------------------

void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing) {
    float texture_size = 1.0/16.0f;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    for(int i=0; i < text.size(); i++) {
        float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
        float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            ((size+spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x + texture_size, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x, texture_y + texture_size,
        }); }
    glUseProgram(program->programID);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}
//-----------------------------------------------------------------------------------------
/*
bool readHeader(std::ifstream &stream) {
    string line;
    mapWidth = -1;
    mapHeight = -1;
    while(getline(stream, line)) {
        if(line == "") { break; }
        
        istringstream sStream(line);
        string key, value;
        getline(sStream, key, '=');
        getline(sStream, value);
        
        if(key == "width") {
            mapWidth = atoi(value.c_str());
        } else if(key == "height"){
            mapHeight = atoi(value.c_str());
        }
    }
    
    if(mapWidth == -1 || mapHeight == -1) {
        return false;
    } else { // allocate our map data
        levelData = new unsigned char*[mapHeight];
        for(int i = 0; i < mapHeight; ++i) {
            levelData[i] = new unsigned char[mapWidth];
        }
        return true;
    }
}

//-----------------------------------------------------------------------------------------

bool readLayerData(std::ifstream &stream) {
    string line;
    while(getline(stream, line)) {
        if(line == "") { break; }
        
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        
        if(key == "data") {
            for(int y=0; y < mapHeight; y++) {
                getline(stream, line);
                istringstream lineStream(line);
                string tile;
                
                for(int x=0; x < mapWidth; x++) {
                    getline(lineStream, tile, ',');
                    unsigned char val =  (unsigned char)atoi(tile.c_str());
                    if(val > 0) {
                        // be careful, the tiles in this format are indexed from 1 not 0
                        levelData[y][x] = val-1;
                    } else {
                        levelData[y][x] = 0;
                    }
                }
            }
        }
    }
    return true;
}

//-----------------------------------------------------------------------------------------

bool readEntityData(std::ifstream &stream) {
    
    string line;
    string type;
    
    while(getline(stream, line)) {
        if(line == "") { break; }
        
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        
        if(key == "type") {
            type = value;
        } else if(key == "location") {
            
            istringstream lineStream(value);
            string xPosition, yPosition;
            getline(lineStream, xPosition, ',');
            getline(lineStream, yPosition, ',');
                    
                    float placeX = atoi(xPosition.c_str())*TILE_SIZE;
                    float placeY = atoi(yPosition.c_str())*-TILE_SIZE;
            
                    //placeEntity(type, placeX, placeY);
                }
            }
            return true;
        }
*/
//-----------------------------------------------------------------------------------------

class Vector3 {
public:
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    
    float x;
    float y;
    float z;
};

class Entity {
public:
    Entity(Vector3 position, Vector3 velocity, Vector3 size) : position(position), velocity(velocity), size(size) {}
    
    Vector3 position;
    Vector3 velocity;
    Vector3 size;
};

class Platform {
public:
    Platform(Vector3 position, Vector3 size) : position(position), size(size) {}
    
    Vector3 position;
    Vector3 size;
};

class Item {
public:
    Item(Vector3 position, Vector3 size, bool alive): position(position), size(size), alive(alive) {}
    
    Vector3 position;
    Vector3 size;
    bool alive;
};

//-----------------------------------------------------------------------------------------

float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}

//-----------------------------------------------------------------------------------------

bool collision(Item one, Entity two){
    if(one.position.y - one.size.y/2 >= two.position.y + two.size.y/2) return true;
    if(one.position.y + one.size.y/2 <= two.position.y - two.size.y/2) return true;
    if(one.position.x - one.size.x/2 >= two.position.x + two.size.x/2) return true;
    if(one.position.x + one.size.x/2 <= two.position.x - two.size.x/2) return true;
    return false;
}

bool collisionYBot(Platform world, Entity player){
    float playerBot = player.position.y - player.size.y/2;
    float playerRight = player.position.x + player.size.x/2;
    float playerLeft = player.position.x - player.size.x/2;
    float playerTop = player.position.y + player.size.y/2;
    float worldTop = world.position.y + world.size.y/2;
    float worldRight = world.position.x + world.size.x/2;
    float worldLeft = world.position.x - world.size.x/2;
    float worldBot = world.position.y - world.size.y/2;
    
    if(playerBot >= worldTop)   return true;
    if(playerTop <= worldBot)   return true;
    if(playerLeft >= worldRight) return true;
    if(playerRight <= worldLeft) return true;
    
    //if(playerBot <= worldTop && playerRight <= worldRight && playerLeft >= worldLeft && playerTop > worldTop) {
    if(playerBot <= worldTop && playerTop > worldTop) {
        y_distance = player.position.y - world.position.y;
        penetration = fabs(y_distance - player.size.y/2 - world.size.y/2);
        adjustY = penetration;
        onFloor = true;
        return false;
    }
    return true;
}

bool collisionXRight(Platform world, Entity player){
    float playerBot = player.position.y - player.size.y/2;
    float playerRight = player.position.x + player.size.x/2;
    float playerLeft = player.position.x - player.size.x/2;
    float playerTop = player.position.y + player.size.y/2;
    float worldTop = world.position.y + world.size.y/2;
    float worldRight = world.position.x + world.size.x/2;
    float worldLeft = world.position.x - world.size.x/2;
    float worldBot = world.position.y - world.size.y/2;
    
    if(playerBot >= worldTop)   return true;
    if(playerTop <= worldBot)   return true;
    if(playerLeft >= worldRight) return true;
    if(playerRight <= worldLeft) return true;
    
    if(playerRight >= worldLeft && playerBot < worldTop) {
        x_distance = player.position.x - world.position.x;
        penetration = fabs(x_distance + player.size.x/2 + world.size.x/2);
        adjustX = penetration;
        return false;
    }
    return true;
}
//-----------------------------------------------------------------------------------------

SDL_Window* displayWindow;

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 576, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    Matrix projectionMatrix;
    Matrix modelMatrixBackGround;
    Matrix modelMatrixGameName;
    Matrix modelMatrixPressButton;
    Matrix modelMatrixPlayer;
    Matrix modelMatrixRuby;
    Matrix modelMatrixScore;
    Matrix viewMatrix;
    
    //Reposition the position of state menu text
    modelMatrixGameName.Translate(-0.5, 0.5, 0.0);
    modelMatrixPressButton.Translate(-2.0, -1.0, 0.0);
    
    //Reposition score;
    modelMatrixScore.Translate(0.5, -9.5, 0.0);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    projectionMatrix.setOrthoProjection(-7.11, 7.11, -4.0f, 4.0f, -1.0f, 1.0f);
    glUseProgram(program.programID);
  
    GLuint spriteSheet = LoadTexture("platformertiles.png");
    GLuint characterSheet = LoadTexture("characters_3.png");
    GLuint fontTexture = LoadTexture("pixel_font.png");
    GLuint rubyTexture = LoadTexture("ruby2.png");
    
    /*
    Mix_Chunk *jumpSound;
    jumpSound = Mix_LoadWAV("jump.wav");
    Mix_Music *music;
    music = Mix_LoadMUS("kh-1-01-Dearly-Beloved.mp3");
    Mix_PlayMusic(music, -1);
    */
    
    //The first tile of spritesheet starts with 0.
    for(int i = 0; i < LEVEL_HEIGHT; i++) {
        for(int j = 0; j < LEVEL_WIDTH; j++){
            levelData[i][j] -= 1;
        }
    }
    
    //Score position
    Vector3 scorePostion(0.5, -9.5, 0.0);
    Vector3 scoreVelocity(0.0f, 0.0f, 0.0f);
    Vector3 scoreSize(0.0f, 0.0f, 0.0f);
    Entity* score_temp = new Entity(scorePostion, scoreVelocity, scoreSize);
    
    //Player
    Vector3 playerPosition(7.25f, -6.75f, 0.0f);
    Vector3 playerVelocity(0.0f, 0.0f, 0.0f);
    Vector3 playerSize(0.5f, 0.5f, 0.0f);
    Entity* player = new Entity(playerPosition, playerVelocity, playerSize);
    
    //Level 1 Platforms
    Vector3 platform1Position(5.5, -9.5, 0.0);
    Vector3 platform1Size(10.5, 5.0, 0.0);
    Platform* platform1 = new Platform(platform1Position, platform1Size);
    
    Vector3 platform2Position(10.85, -6.75, 0.0);
    Vector3 platform2Size(0.5, 0.5, 0.0);
    Platform* platform2 = new Platform(platform2Position, platform2Size);
    
    Vector3 platform3Position(11.35, -6.25, 0.0);
    Vector3 platform3Size(0.5, 0.5, 0.0);
    Platform* platform3 = new Platform(platform3Position, platform3Size);
    
    Vector3 platform4Position(11.75, -5.75, 0.0);
    Vector3 platform4Size(0.3, 0.5, 0.0);
    Platform* platform4 = new Platform(platform4Position, platform4Size);
    
    Vector3 platform5Position(13.25, -5.5, 0.0);
    Vector3 platform5Size(1.0, 1.0, 0.0);
    Platform* platform5 = new Platform(platform5Position, platform5Size);
    
    Vector3 platform6Position(15.25, -5.5, 0.0);
    Vector3 platform6Size(1.0, 1.0, 0.0);
    Platform* platform6 = new Platform(platform6Position, platform6Size);
    
    Vector3 platform7Position(20.75, -9.5, 0.0);
    Vector3 platform7Size(8.7, 5.0, 0.0);
    Platform* platform7 = new Platform(platform7Position, platform7Size);
    
    Vector3 platform8Position(26.16, -6.75, 0.0);
    Vector3 platform8Size(2.1, 0.5, 0.0);
    Platform* platform8 = new Platform(platform8Position, platform8Size);
    
    Vector3 platform9Position(29.2, -6.25, 0.0);
    Vector3 platform9Size(3.2, 0.5, 0.0);
    Platform* platform9 = new Platform(platform9Position, platform9Size);
    
    Vector3 platform10Position(32.45, -5.75, 0.0);
    Vector3 platform10Size(2.5, 0.5, 0.0);
    Platform* platform10 = new Platform(platform10Position, platform10Size);
    
    Vector3 platform11Position(34.2, -5.25, 0.0);
    Vector3 platform11Size(0.15, 0.5, 0.0);
    Platform* platform11 = new Platform(platform11Position, platform11Size);
    
    Vector3 platform12Position(34.70, -4.75, 0.0);
    Vector3 platform12Size(0.15, 0.5, 0.0);
    Platform* platform12 = new Platform(platform12Position, platform12Size);
    
    Vector3 platform13Position(35.45, -4.25, 0.0);
    Vector3 platform13Size(0.65, 0.5, 0.0);
    Platform* platform13 = new Platform(platform13Position, platform13Size);
    
    Vector3 platform14Position(38.7, -8.0, 0.0);
    Vector3 platform14Size(3.15, 7.0, 0.0);
    Platform* platform14 = new Platform(platform14Position, platform14Size);
    
    Vector3 platform15Position(41.5, -4.25, 0.0);
    Vector3 platform15Size(0.65, 0.5, 0.0);
    Platform* platform15 = new Platform(platform15Position, platform15Size);
    
    Vector3 platform16Position(42.7, -3.75, 0.0);
    Vector3 platform16Size(0.2, 0.5, 0.0);
    Platform* platform16 = new Platform(platform16Position, platform16Size);
    
    Vector3 platform17Position(44.2, -3.25, 0.0);
    Vector3 platform17Size(1.1, 0.5, 0.0);
    Platform* platform17 = new Platform(platform17Position, platform17Size);
    
    Vector3 platform18Position(47.25, -2.75, 0.0);
    Vector3 platform18Size(3.2, 0.5, 0.0);
    Platform* platform18 = new Platform(platform18Position, platform18Size);
    
    Vector3 platform19Position(46.2, -5.75, 0.0);
    Vector3 platform19Size(9.1, 0.5, 0.0);
    Platform* platform19 = new Platform(platform19Position, platform19Size);
    
    Vector3 platform20Position(52.2, -9.0, 0.0);
    Vector3 platform20Size(1.15, 5.0, 0.0);
    Platform* platform20 = new Platform(platform20Position, platform20Size);
    
    //Vector contains all platforms player needs to test bottom collision with for level 1
    vector<Platform> Level1Y;
    Level1Y.push_back(*platform1);
    Level1Y.push_back(*platform2);
    Level1Y.push_back(*platform3);
    Level1Y.push_back(*platform4);
    Level1Y.push_back(*platform5);
    Level1Y.push_back(*platform6);
    Level1Y.push_back(*platform7);
    Level1Y.push_back(*platform8);
    Level1Y.push_back(*platform9);
    Level1Y.push_back(*platform10);
    Level1Y.push_back(*platform11);
    Level1Y.push_back(*platform12);
    Level1Y.push_back(*platform13);
    Level1Y.push_back(*platform14);
    Level1Y.push_back(*platform15);
    Level1Y.push_back(*platform16);
    Level1Y.push_back(*platform17);
    Level1Y.push_back(*platform18);
    Level1Y.push_back(*platform19);
    Level1Y.push_back(*platform20);
    
    //Vector contains all platforms player needs to test right collision with for level 1
    vector<Platform> Level1X;
    Level1X.push_back(*platform2);
    Level1X.push_back(*platform3);
    Level1X.push_back(*platform4);
    Level1X.push_back(*platform8);
    Level1X.push_back(*platform9);
    Level1X.push_back(*platform10);
    Level1X.push_back(*platform11);
    Level1X.push_back(*platform12);
    Level1X.push_back(*platform13);
    Level1X.push_back(*platform14);
    Level1X.push_back(*platform15);
    Level1X.push_back(*platform16);
    Level1X.push_back(*platform17);
    Level1X.push_back(*platform18);
    Level1X.push_back(*platform19);
    Level1X.push_back(*platform20);


    //Items
    Vector3 item1Position(12.225, -4.675, 0.0);
    Vector3 item1Size(0.25, 0.25, 0.0);
    Item* item1 = new Item(item1Position, item1Size, true);

    Vector3 item2Position(14.225, -4.2, 0.0);
    Vector3 item2Size(0.25, 0.25, 0.0);
    Item* item2 = new Item(item2Position, item2Size, true);
    
    Vector3 item3Position(16.225, -4.2, 0.0);
    Vector3 item3Size(0.25, 0.25, 0.0);
    Item* item3 = new Item(item3Position, item3Size, true);
    
    Vector3 item4Position(16.625, -4.675, 0.0);
    Vector3 item4Size(0.25, 0.25, 0.0);
    Item* item4 = new Item(item4Position, item4Size, true);
    
    Vector3 item5Position(17.025, -4.975, 0.0);
    Vector3 item5Size(0.25, 0.25, 0.0);
    Item* item5 = new Item(item5Position, item5Size, true);
    
    Vector3 item6Position(25.225, -6.275, 0.0);
    Vector3 item6Size(0.25, 0.25, 0.0);
    Item* item6 = new Item(item6Position, item6Size, true);
    
    Vector3 item7Position(25.625, -6.275, 0.0);
    Vector3 item7Size(0.25, 0.25, 0.0);
    Item* item7 = new Item(item7Position, item7Size, true);
    
    Vector3 item8Position(26.25, -6.275, 0.0);
    Vector3 item8Size(0.25, 0.25, 0.0);
    Item* item8 = new Item(item8Position, item8Size, true);
    
    Vector3 item9Position(26.65, -6.275, 0.0);
    Vector3 item9Size(0.25, 0.25, 0.0);
    Item* item9 = new Item(item9Position, item9Size, true);
    
    Vector3 item10Position(27.05, -6.275, 0.0);
    Vector3 item10Size(0.25, 0.25, 0.0);
    Item* item10 = new Item(item10Position, item10Size, true);
    
    Vector3 item11Position(27.45, -6.275, 0.0);
    Vector3 item11Size(0.25, 0.25, 0.0);
    Item* item11 = new Item(item11Position, item11Size, true);
    
    Vector3 item12Position(36.325, -2.875, 0.0);
    Vector3 item12Size(0.25, 0.25, 0.0);
    Item* item12 = new Item(item12Position, item12Size, true);
    
    Vector3 item13Position(36.825, -3.275, 0.0);
    Vector3 item13Size(0.25, 0.25, 0.0);
    Item* item13 = new Item(item13Position, item13Size, true);
    
    Vector3 item14Position(37.225, -3.675, 0.0);
    Vector3 item14Size(0.25, 0.25, 0.0);
    Item* item14 = new Item(item14Position, item14Size, true);
    
    
    /*
    ifstream infile("Level1.txt");
    string line;
    while (getline(infile, line)) {
        // handle line
        if (line == "[header]") {
            if (!readHeader(infile)) {
                return 0;
            }
        }
        else if (line == "[layer]") {
            readLayerData(infile);
        }
        else if (line == "[Objects]") {
            readEntityData(infile);
        }
    }*/
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }else if(event.type == SDL_KEYDOWN && state == STATE_MAIN_MENU) {
                state = STATE_GAME_LEVEL1;
            }else if(event.type == SDL_KEYDOWN && state == STATE_GAME_LEVEL1) {
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                    moveJump = true;
                    //Mix_PlayChannel(-1, jumpSound, 0);
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
                    moveDown = true;
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
                    moveLeft = true;
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
                    moveRight = true;
                }
            }
        }
        
        //Loop
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClear(GL_COLOR_BUFFER_BIT);
        
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        float fixedElapsed = elapsed;
        if(fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
            fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
        }
        while (fixedElapsed >= FIXED_TIMESTEP ) {
            fixedElapsed -= FIXED_TIMESTEP;
        }
        

        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        
    
        switch(state) {
            case STATE_MAIN_MENU:
                
                //Game Name Texture
                program.setModelMatrix(modelMatrixGameName);
                DrawText(&program, fontTexture, "Game", 0.3, 0);
                
                //Press Button Texture
                program.setModelMatrix(modelMatrixPressButton);
                DrawText(&program, fontTexture, "Press any key to start!", 0.2, 0);
                
                break;
                
            case STATE_GAME_LEVEL1:
                
                //Draw world
                program.setModelMatrix(modelMatrixBackGround);
                DrawWorld(&program, spriteSheet);
                
                
                //Player setup
                float playerVertices[] = {7.0, -7.0, 7.5, -6.5, 7.0, -6.5, 7.5, -6.5, 7.0, -7.0, 7.5, -7.0};
                program.setModelMatrix(modelMatrixPlayer);
                
                //Player walk animation
                animationElapsed += elapsed * 5;
                if(moveRight && onFloor && animationElapsed > 1.0/framesPerSecond) {
                    currentIndex++;
                    animationElapsed = 0.0;
                    if(currentIndex > numFrames-1) {
                        currentIndex = 0;
                    }
                }
                
                DrawSpriteSheetSprite(&program, characterSheet, playerVertices, runAnimation[currentIndex], 8, 4);
                //&& onFloor
                if(moveJump) {
                    onFloor = false;
                    accelerationY = 6.0;
                    player->velocity.y += accelerationY * FIXED_TIMESTEP;
                }else if(moveDown) {
                    accelerationY = 1.0f;
                    player->velocity.y -= accelerationY * FIXED_TIMESTEP;
                }else if(moveLeft) {
                    player->velocity.x -= accelerationX * FIXED_TIMESTEP;
                }else if(moveRight) {
                    player->velocity.x += accelerationX * FIXED_TIMESTEP;
                }
                
                modelMatrixPlayer.Translate(0.0, -gravity * FIXED_TIMESTEP, 0.0);
                player->position.y -= gravity * FIXED_TIMESTEP;
                
                player->velocity.y = lerp(player->velocity.y, 0.0f, FIXED_TIMESTEP * friction);
                player->velocity.x = lerp(player->velocity.x, 0.0f, FIXED_TIMESTEP * friction);
                
                modelMatrixPlayer.Translate(player->velocity.x, player->velocity.y, 0.0);
                moveJump = false;
                moveDown = false;
                moveLeft = false;
                moveRight = false;
                
                player->position.y += player->velocity.y;
               
                //Check PlayerBot and WorldTop collision for each platform
                for(int i = 0; i < Level1Y.size(); i++) {
                    if(!collisionYBot(Level1Y[i], *player)) {
                        modelMatrixPlayer.Translate(0.0, adjustY + 0.01, 0.0);
                        player->velocity.y = 0.0f;
                        player->position.y += adjustY + 0.01;
                    }
                }
                
                player->position.x += player->velocity.x;
        
                //Check PlayerRight and WorldLeft collision for each platform
                for(int j = 0; j < Level1X.size(); j++) {
                    if(!collisionXRight(Level1X[j], *player)) {
                        modelMatrixPlayer.Translate(-adjustX + 0.01, 0.0, 0.0);
                        player->velocity.x = 0.0f;
                        player->position.x -= adjustX + 0.01;
                    }
                }

                cout << "X: " << player->position.x << " Y: " << player->position.y << endl;
                
                //Ruby setup
                program.setModelMatrix(modelMatrixRuby);
                
                animationElapsedRuby += elapsed * 0.5;
                if(animationElapsedRuby > 1.0/framesPerSecond) {
                    currentIndexRuby++;
                    animationElapsedRuby = 0.0;
                    if(currentIndexRuby > numFramesRuby-1) {
                        currentIndexRuby = 0;
                    }
                }
        
                //---
                float rubyVertices1[] = {12.1, -4.8, 12.35, -4.55, 12.1, -4.55, 12.35, -4.55, 12.1, -4.8, 12.35, -4.8};
                if(item1->alive != false) {
                    DrawSpriteSheetSprite(&program, rubyTexture, rubyVertices1, runAnimationRuby[currentIndexRuby], 7, 1);
                }
                
                if(!collision(*item1, *player) && item1->alive) {
                    item1->alive = false;
                    score += 100;
                }
                
                //-----
                float rubyVertices2[] = {14.1, -4.3, 14.35, -4.05, 14.1, -4.05, 14.35, -4.05, 14.1, -4.3, 14.35, -4.3};
                if(item2->alive != false) {
                    DrawSpriteSheetSprite(&program, rubyTexture, rubyVertices2, runAnimationRuby[currentIndexRuby], 7, 1);
                }
                
                if(!collision(*item2, *player) && item2->alive) {
                    item2->alive = false;
                    score += 100;
                }
                //-----
                
                float rubyVertices3[] = {16.1, -4.3, 16.35, -4.05, 16.1, -4.05, 16.35, -4.05, 16.1, -4.3, 16.35, -4.3};
                if(item3->alive != false) {
                    DrawSpriteSheetSprite(&program, rubyTexture, rubyVertices3, runAnimationRuby[currentIndexRuby], 7, 1);
                }
                
                if(!collision(*item3, *player) && item3->alive) {
                    item3->alive = false;
                    score += 100;
                }
                
                //----
                float rubyVertices4[] = {16.5, -4.7, 16.75, -4.45, 16.5, -4.45, 16.75, -4.45, 16.5, -4.7, 16.75, -4.7};
                if(item4->alive != false) {
                    DrawSpriteSheetSprite(&program, rubyTexture, rubyVertices4, runAnimationRuby[currentIndexRuby], 7, 1);
                }
                
                if(!collision(*item4, *player) && item4->alive) {
                    item4->alive = false;
                    score += 100;
                }
                
                //----
                float rubyVertices5[] = {16.9, -5.1, 17.15, -4.85, 16.9, -4.85, 17.15, -4.85, 16.9, -5.1, 17.15, -5.1};
                if(item5->alive != false) {
                    DrawSpriteSheetSprite(&program, rubyTexture, rubyVertices5, runAnimationRuby[currentIndexRuby], 7, 1);
                }
                
                if(!collision(*item5, *player) && item5->alive) {
                    item5->alive = false;
                    score += 100;
                }
                
                //----
                float rubyVertices6[] = {25.1, -6.4, 25.35, -6.15, 25.1, -6.15, 25.35, -6.15, 25.1, -6.4, 25.35, -6.4};
                if(item6->alive != false) {
                    DrawSpriteSheetSprite(&program, rubyTexture, rubyVertices6, runAnimationRuby[currentIndexRuby], 7, 1);
                }
                
                if(!collision(*item6, *player) && item6->alive) {
                    item6->alive = false;
                    score += 100;
                }
                
                //----
                float rubyVertices7[] = {25.5, -6.4, 25.75, -6.15, 25.5, -6.15, 25.75, -6.15, 25.5, -6.4, 25.75, -6.4};
                if(item7->alive != false) {
                    DrawSpriteSheetSprite(&program, rubyTexture, rubyVertices7, runAnimationRuby[currentIndexRuby], 7, 1);
                }
                
                if(!collision(*item7, *player) && item7->alive) {
                    item7->alive = false;
                    score += 100;
                }
                
                //----
                float rubyVertices8[] = {25.9, -6.4, 26.15, -6.15, 25.9, -6.15, 26.15, -6.15, 25.9, -6.4, 26.15, -6.4};
                if(item8->alive != false) {
                    DrawSpriteSheetSprite(&program, rubyTexture, rubyVertices8, runAnimationRuby[currentIndexRuby], 7, 1);
                }
                
                if(!collision(*item8, *player) && item8->alive) {
                    item8->alive = false;
                    score += 100;
                }
                
                //----
                float rubyVertices9[] = {26.3, -6.4, 26.55, -6.15, 26.3, -6.15, 26.55, -6.15, 26.3, -6.4, 26.55, -6.4};
                if(item9->alive != false) {
                    DrawSpriteSheetSprite(&program, rubyTexture, rubyVertices9, runAnimationRuby[currentIndexRuby], 7, 1);
                }
                
                if(!collision(*item9, *player) && item9->alive) {
                    item9->alive = false;
                    score += 100;
                }
                
                //----
                float rubyVertices10[] = {26.7, -6.4, 26.95, -6.15, 26.7, -6.15, 26.95, -6.15, 26.7, -6.4, 26.95, -6.4};
                if(item10->alive != false) {
                    DrawSpriteSheetSprite(&program, rubyTexture, rubyVertices10, runAnimationRuby[currentIndexRuby], 7, 1);
                }
                
                if(!collision(*item10, *player) && item10->alive) {
                    item10->alive = false;
                    score += 100;
                }
                
                //----
                float rubyVertices11[] = {27.1, -6.4, 27.35, -6.15, 27.1, -6.15, 27.35, -6.15, 27.1, -6.4, 27.35, -6.4};
                if(item11->alive != false) {
                    DrawSpriteSheetSprite(&program, rubyTexture, rubyVertices11, runAnimationRuby[currentIndexRuby], 7, 1);
                }
                
                if(!collision(*item11, *player) && item11->alive) {
                    item11->alive = false;
                    score += 100;
                }
                
                //----
                float rubyVertices12[] = {36.2, -3.0, 36.45, -2.75, 36.2, -2.75, 36.45, -2.75, 36.2, -3.0, 36.45, -3.0};
                if(item12->alive != false) {
                    DrawSpriteSheetSprite(&program, rubyTexture, rubyVertices12, runAnimationRuby[currentIndexRuby], 7, 1);
                }
                
                if(!collision(*item12, *player) && item12->alive) {
                    item12->alive = false;
                    score += 100;
                }
                
                //----
                float rubyVertices13[] = {36.7, -3.4, 36.95, -3.15, 36.7, -3.15, 36.95, -3.15, 36.7, -3.4, 36.95, -3.4};
                if(item13->alive != false) {
                    DrawSpriteSheetSprite(&program, rubyTexture, rubyVertices13, runAnimationRuby[currentIndexRuby], 7, 1);
                }
                
                if(!collision(*item13, *player) && item13->alive) {
                    item13->alive = false;
                    score += 100;
                }
                
                //----
                float rubyVertices14[] = {37.1, -3.8, 37.35, -3.55, 37.1, -3.55, 37.35, -3.55, 37.1, -3.8, 37.35, -3.8};
                if(item14->alive != false) {
                    DrawSpriteSheetSprite(&program, rubyTexture, rubyVertices14, runAnimationRuby[currentIndexRuby], 7, 1);
                }
                
                if(!collision(*item14, *player) && item14->alive) {
                    item14->alive = false;
                    score += 100;
                }
                
                //Score matrix
                program.setModelMatrix(modelMatrixScore);
                
                if(score_temp->position.y <= -9.5) {
                    //do noting
                }
                else {
                    modelMatrixScore.Translate(0.0, -gravity * FIXED_TIMESTEP, 0.0);
                    score_temp->position.y -= gravity * FIXED_TIMESTEP;
                }
                
                modelMatrixScore.Translate(player->velocity.x, player->velocity.y, 0.0);
                
                score_temp->position.x += player->velocity.x;
                score_temp->position.y += player->velocity.y;
                
                //cout << "ScX: " << score_temp->position.x << " ScY: " << score_temp->position.y;
                DrawText(&program, fontTexture, "Score: "+to_string(score), 0.2, 0.0);
        
                //Scrolling view matrix
                viewMatrix.identity();
                viewMatrix.Translate(-player->position.x, -player->position.y-1, 0);
                program.setViewMatrix(viewMatrix);
                
            break;

            /*
            case STATE_GAME_END:
                
                modelScore.Translate(-1.2, 0.0, 0.0);
                //program.setModelMatrix(modelScore);
                DrawText(&program, fontTexture, "Game Over!", 0.3, 0.0);
                
            break;
             */
        }
        
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    delete player;
    delete score_temp;
    delete platform1;
    delete platform2;
    delete platform3;
    delete platform4;
    delete platform5;
    delete platform6;
    delete platform8;
    delete platform9;
    delete platform10;
    delete platform11;
    delete platform12;
    delete platform13;
    delete platform14;
    delete platform15;
    delete platform16;
    delete platform17;
    delete platform18;
    delete platform19;
    delete item1;
    delete item2;
    delete item3;
    delete item4;
    delete item5;
    delete item6;
    delete item7;
    delete item8;
    delete item9;
    delete item10;
    delete item11;
    delete item12;
    delete item13;
    delete item14;
    
    SDL_Quit();
    return 0;
}
