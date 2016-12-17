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

float friction = 2.0;
float accelerationX = 3.0f, accelerationY = 3.0f;
bool moveJump = false, moveDown = false, moveLeft = false, moveRight = false;
bool idle = false;
//unsigned char** levelData;


enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL1, STATEs_GAME_LEVEL2, STATE_GAME_LEVEL3, STATE_GAME_END };
int state = STATE_MAIN_MENU;

float lastFrameTicks = 0.0f;
const Uint8 *keys = SDL_GetKeyboardState(NULL);


unsigned char levelData[LEVEL_HEIGHT][LEVEL_WIDTH] =
{
    {12,12,12,12,12,12,12,12,12,12,12,12,12,12,16,12,12,12,12,12,14,12,12,12,12,12,12,12,12,12,12,12,12,16,12,12,12,12,12,12,12,12,12,12,12,12,12,14,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,14,12,12,12,14,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,15,12,12,12,12,12,12,12,22,15,5,22,22,12,16,12,16,12,12,12,15,23,23,23,15,12,12},
    {12,15,14,12,12,12,12,12,12,12,14,12,14,12,12,12,12,12,12,12,12,12,12,12,14,12,12,12,15,12,15,12,12,12,12,12,14,12,12,12,12,12,16,12,12,12,12,12,12,12,12,12,12,16,12,12,12,12,12,12,16,12,12,12,12,12,12,12,12,14,12,12,12,16,12,12,12,12,12,12,12,12,12,12,12,12,14,12,12,12,12,16,12,12,12,15,12,12,12,15,12,12,12,12,12,12,14,12,14,12,1,2,2,7,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {14,12,12,15,22,22,22,15,12,12,12,12,12,12,12,12,12,16,12,12,12,12,12,12,12,12,12,12,12,12,12,12,14,12,12,12,12,12,12,14,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,14,12,12,12,12,12,12,12,12,12,14,12,15,12,12,12,16,12,12,12,12,12,12,12,12,12,12,12,12,14,12,12,14,12,12,12,12,12,12,22,12,12,12,12,12,12,14,12,12,12,12,12,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {12,12,1,2,2,2,2,2,3,12,12,12,12,12,12,12,12,12,12,12,16,12,12,12,12,12,16,12,12,15,12,12,12,12,12,12,12,12,12,12,12,12,12,12,16,12,12,12,12,12,15,15,12,12,12,12,12,12,12,12,12,12,12,16,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,14,12,12,14,12,12,12,12,12,12,12,16,12,12,23,5,23,12,12,12,12,12,12,12,12,14,12,12,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {12,12,9,10,10,10,10,10,11,12,16,12,12,12,12,14,15,12,12,12,12,12,12,14,12,12,12,12,12,12,12,12,12,12,15,12,12,15,12,12,12,12,12,14,12,12,12,12,15,15,12,12,12,12,12,12,16,12,12,12,14,22,12,12,12,12,12,12,12,12,12,12,12,16,12,12,12,12,12,12,12,12,12,12,12,12,12,12,22,12,12,12,1,2,7,2,2,3,12,12,12,12,14,12,12,12,12,12,4,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {12,12,9,10,10,10,10,10,11,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,14,12,12,12,12,12,12,12,12,12,12,12,23,12,12,12,12,12,12,12,12,12,12,12,12,12,15,12,12,12,12,12,12,5,12,12,12,12,12,12,15,12,12,12,12,16,12,12,14,12,14,12,12,12,12,0,22,22,12,1,2,3,12,12,17,18,18,18,18,19,12,12,16,12,12,12,12,12,15,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {12,12,9,10,10,10,10,10,11,12,12,12,12,16,12,12,12,12,12,12,12,15,12,12,16,12,4,12,12,12,12,12,15,12,12,12,12,12,12,15,5,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,13,14,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,14,12,12,1,2,3,12,17,18,19,12,12,24,12,12,24,24,24,12,12,12,12,12,15,12,12,22,22,22,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {12,12,9,10,10,10,10,10,11,12,12,12,12,12,12,12,16,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,14,12,23,12,13,22,22,12,12,14,12,12,12,12,12,12,12,12,12,12,12,12,23,22,22,13,12,22,12,12,12,12,12,12,12,22,12,12,12,12,12,12,12,12,12,12,12,17,18,19,12,12,24,24,12,12,16,12,12,14,12,12,14,12,12,12,12,12,12,1,2,2,2,2,8,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {22,22,9,10,10,10,10,10,11,12,12,23,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,1,2,7,2,2,3,12,12,12,12,12,12,12,23,12,12,12,12,12,12,1,2,2,7,2,3,12,12,12,16,12,23,1,3,12,12,22,12,12,12,12,12,22,12,12,24,14,12,12,12,14,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {2,2,2,2,2,2,2,2,2,2,2,3,12,12,12,12,12,12,12,14,15,12,12,12,12,22,22,22,12,22,22,22,12,12,12,12,12,12,9,10,10,10,10,11,12,12,12,14,12,12,12,1,2,3,12,12,12,12,9,10,10,10,10,11,12,12,12,12,23,1,8,11,15,12,1,2,2,2,2,2,3,12,12,12,12,12,12,12,12,14,12,12,12,12,12,14,12,12,12,12,12,12,15,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,11,12,15,12,12,12,16,12,12,12,12,12,23,12,1,2,3,12,1,2,3,12,12,12,15,12,12,9,10,10,10,10,11,12,12,12,12,12,12,12,17,18,19,12,12,12,12,9,10,10,10,10,11,12,12,12,23,1,8,10,11,12,12,9,10,10,10,10,10,11,12,12,22,22,22,12,12,22,22,22,23,22,12,12,12,12,12,12,23,22,22,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,11,12,12,12,12,12,12,12,16,12,12,12,5,12,17,18,19,12,17,18,19,12,12,12,12,12,12,9,10,10,10,10,11,12,12,12,12,12,15,12,12,24,12,12,12,22,22,9,10,10,10,1,2,2,2,2,2,8,10,10,11,12,12,9,10,10,10,10,10,11,12,12,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,12,12,16,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,11,12,12,12,14,12,12,12,12,12,12,5,13,12,24,24,15,12,24,24,24,12,12,23,12,12,12,9,10,10,10,10,11,12,23,12,12,12,12,23,22,12,12,12,1,2,2,2,2,2,2,8,10,10,10,10,10,10,10,10,11,15,12,9,10,10,10,10,10,11,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,22,23,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,11,22,22,12,12,12,12,12,12,23,5,13,13,23,12,12,12,12,12,12,12,12,22,5,12,12,12,9,10,10,10,10,11,22,5,12,12,12,23,1,2,2,2,2,8,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,9,10,10,10,10,10,11,12,12,17,18,18,18,10,10,10,10,10,10,10,10,10,10,10,18,18,18,19,12,1,2,3,20,20,20,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,7,7,7,3,16,12,12,12,12,12,12,12,1,7,2,2,2,2,2,2,2,2,2,2,7,2,2,2,2,8,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,9,10,10,10,10,10,11,12,12,24,24,12,12,9,10,10,10,10,10,10,10,10,10,11,12,24,24,24,12,9,10,11,20,20,20,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,15,12,12,12,15,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,15,9,10,10,10,10,10,11,15,12,12,14,12,14,9,10,10,10,10,10,10,10,10,10,11,14,12,12,12,12,9,10,11,20,20,20,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,15,12,12,12,12,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,9,10,10,10,10,10,11,12,12,14,12,12,12,9,10,10,10,10,10,10,10,10,10,11,12,14,12,12,12,9,10,11,20,20,20,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,16,12,12,14,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,9,10,10,10,10,10,11,12,12,14,12,14,12,9,10,10,10,10,10,10,10,10,10,11,12,14,12,15,12,9,10,11,20,20,20,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,12,12,12,15,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,15,12,9,10,10,10,10,10,11,15,12,12,12,12,14,9,10,10,10,10,10,10,10,10,10,11,12,12,12,12,12,9,10,11,20,20,20,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,16,12,12,12,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,9,10,10,10,10,10,11,12,14,12,12,12,12,9,10,10,10,10,10,10,10,10,10,11,12,12,12,12,15,9,10,11,20,20,20,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,16,12,12,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,9,10,10,10,10,10,11,12,12,12,12,15,12,9,10,10,10,10,10,10,10,10,10,11,12,15,12,12,12,9,10,11,20,20,20,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,12,12,12,12,15,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,9,10,10,10,10,10,11,12,12,12,12,12,12,9,10,10,10,10,10,10,10,10,10,11,12,12,12,12,12,9,10,11,20,20,20,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,12,12,12,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,9,10,10,10,10,10,11,12,12,15,12,12,12,9,10,10,10,10,10,10,10,10,10,11,12,12,15,12,12,9,10,11,20,20,20,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,14,12,12,15,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,9,10,10,10,10,10,11,12,12,12,12,12,12,9,10,10,10,10,10,10,10,10,10,11,12,12,12,12,15,9,10,11,20,20,20,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10},
    {10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,15,12,12,12,12,12,12,12,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,12,12,9,10,10,10,10,10,11,12,12,12,12,14,12,9,10,10,10,10,10,10,10,10,10,11,12,12,12,12,12,9,10,11,20,20,20,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10}
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

void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
    *gridX = (int)(worldX / TILE_SIZE);
    *gridY = (int)(-worldY / TILE_SIZE);
}

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

//-----------------------------------------------------------------------------------------

float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
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
    Matrix modelMatrixPlayer;
    Matrix viewMatrix;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    projectionMatrix.setOrthoProjection(-7.11, 7.11, -4.0f, 4.0f, -1.0f, 1.0f);
    glUseProgram(program.programID);
  
    GLuint spriteSheet = LoadTexture("platformertiles.png");
    GLuint characterSheet = LoadTexture("characters_3.png");
    GLuint fontTexture = LoadTexture("font1.png");
    
    /*
    Mix_Chunk *jumpSound;
    jumpSound = Mix_LoadWAV("jump.wav");
    Mix_Music *music;
    music = Mix_LoadMUS("kh-1-01-Dearly-Beloved.mp3");
    Mix_PlayMusic(music, -1);
    */
    
    //The first tile of my spritesheet started with 0.
    for(int i = 0; i < LEVEL_HEIGHT; i++) {
        for(int j = 0; j < LEVEL_WIDTH; j++){
            levelData[i][j] -= 1;
        }
    }
    
    Vector3 playerPosition(2.25f, -6.75f, 0.0f);
    Vector3 playerVelocity(0.0f, 0.0f, 0.0f);
    Vector3 playerSize(0.5f, 0.5f, 0.0f);
    Entity* player = new Entity(playerPosition, playerVelocity, playerSize);
    
    viewMatrix.Translate(-7.0, 5.0, 0.0);
    
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
            }else if(event.type == SDL_KEYDOWN) {
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
        
    //Draw world
        program.setModelMatrix(modelMatrixBackGround);
        DrawWorld(&program, spriteSheet);
        
        
    //Player setup
        float playerVertices[] = {2.0, -7.0, 2.5, -6.5, 2.0, -6.5, 2.5, -6.5, 2.0, -7.0, 2.5, -7.0};
        program.setModelMatrix(modelMatrixPlayer);
        //DrawSpriteSheetSprite(&program, characterSheet, playerVertices, 9, 8, 4);
        
        animationElapsed += elapsed * 5;
        if(moveRight && animationElapsed > 1.0/framesPerSecond) {
            currentIndex++;
            animationElapsed = 0.0;
            if(currentIndex > numFrames-1) {
                currentIndex = 0;
            }
        }

        DrawSpriteSheetSprite(&program, characterSheet, playerVertices, runAnimation[currentIndex], 8, 4);
        
        if(moveJump == true) {
            accelerationY = 14.0;
            player->velocity.y += accelerationY * FIXED_TIMESTEP;
        }else if(moveDown == true) {
            accelerationY = 3.0f;
            player->velocity.y -= accelerationY * FIXED_TIMESTEP;
        }else if(moveLeft == true) {
            player->velocity.x -= accelerationX * FIXED_TIMESTEP;
        }else if(moveRight == true) {
            player->velocity.x += accelerationX * FIXED_TIMESTEP;
        }
        
        player->velocity.y = lerp(player->velocity.y, 0.0f, FIXED_TIMESTEP * friction);
        player->velocity.x = lerp(player->velocity.x, 0.0f, FIXED_TIMESTEP * friction);
        
        modelMatrixPlayer.Translate(player->velocity.x, player->velocity.y, 0.0);
        moveJump = false;
        moveDown = false;
        moveLeft = false;
        moveRight = false;
        
        player->position.y += player->velocity.y;
        player->position.x += player->velocity.x;
        
        viewMatrix.identity();
        viewMatrix.Translate(-player->position.x, -player->position.y, 0);
        program.setViewMatrix(viewMatrix);
        
        /*
        switch(state) {
            case STATE_MAIN_MENU:
                
                program.setModelMatrix(modelMatrix);
                DrawWorld(program.programID, spriteSheet);
                
            break;
                
            case STATE_GAME_LEVEL:
                
            break;
                
            case STATE_GAME_END:
                
            break;
        }
         */
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    delete player;
    
    SDL_Quit();
    return 0;
}
