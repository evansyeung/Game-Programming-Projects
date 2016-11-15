/*
 Evans Yeung
 Platformer
 
 There are 3 platforms for the player to jump on. 
 I was not sure how to get the player scrolling to work.
 The view has to be manually adjusted to see the last world platform on the right. 
 Player can sometimes sink through static tiles while on elevated tiles. While on the lowest level tiles, player will stay above tile.
 X collision was not fully implemented.
 Sound was implemented for background music and jump.
 */


#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include "ShaderProgram.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

float accelerationX = 3.0f, accelerationY = 3.0f;
float enemyaccelerationX = 0.01f;
float friction = 2.0;
bool moveSPACE = false, moveDOWN = false, moveLEFT = false, moveRIGHT = false;
float adjustY = 0.0f, adjustX = 0.0f;
bool top = false, bottom = false, left = false, right = false;
float y_distance = 0.0f, x_distance = 0.0f;
float gravity = 3.0f;
float penetration = 0.0f;

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

class Vector3 {
public:
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    
    float x;
    float y;
    float z;
};

class Entity {
public:
    Entity(Vector3 position, Vector3 velocity, Vector3 size, bool dead) : position(position), velocity(velocity), size(size), dynamic(NULL) {}
    
    Vector3 position;
    Vector3 velocity;
    Vector3 size;
    
    bool dynamic = false;
};

bool collision(Entity world, Entity player, bool checkY, bool checkXRight){
    if(player.position.y - player.size.y/2 >= world.position.y + world.size.y/2)   return true;
    if(player.position.y + player.size.y/2 <= world.position.y - world.size.y/2)   return true;
    if(player.position.x - player.size.x/2 >= world.position.x + world.size.x/2) return true;
    if(player.position.x + player.size.x/2 <= world.position.x - world.size.x/2) return true;
    
    float playerTop = player.position.y + player.size.x;
    float playerBot = player.position.y - player.size.y;
    //float playerLeft = player.position.x - player.size.x;
    float playerRight = player.position.x + player.size.x;

    float worldTop = world.position.y + world.size.y;
    //float worldRight = world.position.x - world.size.x;
    float worldLeft = world.position.x + world.size.x;

    if(checkY) {
        //std::cout << "COLLISION Y\n";
        player.position.y -= 0.5;
    }
    if(playerBot <= worldTop && playerTop > worldTop + 0.9 &&checkY) {
        player.position.y  += 0.5;
        player.velocity.x = 0.0f;
        player.velocity.y = 0.0f;
        y_distance = player.position.y  - world.position.y;
        penetration = fabs(y_distance - player.size.y/2 - world.size.y/2);
        adjustY = penetration;
    }
    if(checkXRight) {
        //std::cout << "COLLISION X\n";
        player.position.x += 0.5;
    }
    if(playerRight > worldLeft && checkXRight && !checkY) {
        player.position.x -= 0.5;
        player.velocity.x = 0.0f;
        player.velocity.y = 0.0f;
        x_distance = world.position.x - player.position.x;
        penetration = fabs(x_distance - player.size.x/2 - world.size.x/2);
        adjustX = penetration;
    }
    checkY = false;
    return false;
}

float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}

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
    Matrix modelMatrix;
    Matrix modelEnemy;
    Matrix modelWorld;
    Matrix viewMatrix;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    projectionMatrix.setOrthoProjection(-8.88, 8.88, -5.0f, 5.0f, -1.0f, 1.0f);
    glUseProgram(program.programID);

    GLuint spritesheet = LoadTexture("arne_sprites.png");
    Mix_Chunk *jumpSound;
    jumpSound = Mix_LoadWAV("jump.wav");
    Mix_Music *music;
    music = Mix_LoadMUS("kh-1-01-Dearly-Beloved.mp3");
    Mix_PlayMusic(music, -1);
    
    float lastFrameTicks = 0.0f;
    
    //Player setup
    Vector3 playerPosition(1.5f, -5.5f, 0.0f);
    Vector3 playerVelocity(0.0f, 0.0f, 0.0f);
    Vector3 playerSize(1.0f, 1.0f, 0.0f);
    Entity* player = new Entity(playerPosition, playerVelocity, playerSize, true);
    
    //Enemy setup
    Vector3 enemyPosition(9.5f, -8.5f, 0.0f);
    Vector3 enemyVelocity(0.0f, 0.0f, 0.0f);
    Vector3 enemySize(1.0f, 1.0f, 0.0f);
    Entity* enemy1 = new Entity(enemyPosition, enemyVelocity, enemySize, true);
    
    //Platform1 setup
    Vector3 plat1Position(3.0f, -9.5f, 0.0f);
    Vector3 plat1Velocity(0.0f, 0.0f, 0.0f);
    Vector3 plat1Size(4.0f, 1.0f, 0.0f);
    Entity* platform1 = new Entity(plat1Position, plat1Velocity, plat1Size, false);
    
    //Block1 setup
    Vector3 block1Position(4.5f, -8.5f, 0.0f);
    Vector3 block1Velocity(0.0f, 0.0f, 0.0f);
    Vector3 block1Size(1.0f, 1.0f, 0.0f);
    Entity* block1 = new Entity(block1Position, block1Velocity, block1Size, false);
    
    //Block2 setup (fist 3 vertical tiles)
    Vector3 block2Position(7.5f, -8.5f, 0.0f);
    Vector3 block2Velocity(0.0f, 0.0f, 0.0f);
    Vector3 block2Size(1.0f, 3.0f, 0.0f);
    Entity* block2 = new Entity(block2Position, block2Velocity, block2Size, false);
    
    //Block3 setup (first 2 vertical tiles)
    Vector3 block3Position(8.5f, -9.0f, 0.0f);
    Vector3 block3Velocity(0.0f, 0.0f, 0.0f);
    Vector3 block3Size(1.0f, 2.0f, 0.0f);
    Entity* block3 = new Entity(block3Position, block3Velocity, block3Size, false);
    
    //Platform2 setup
    Vector3 plat2Position(12.0f, -9.5f, 0.0f);
    Vector3 plat2Velocity(0.0f, 0.0f, 0.0f);
    Vector3 plat2Size(6.0f, 1.0f, 0.0f);
    Entity* platform2 = new Entity(plat2Position, plat2Velocity, plat2Size, false);
    
    //Block4 setup (second 2 vertical tiles)
    Vector3 block4Position(15.5f, -9.0f, 0.0f);
    Vector3 block4Velocity(0.0f, 0.0f, 0.0f);
    Vector3 block4Size(1.0f, 2.0f, 0.0f);
    Entity* block4 = new Entity(block4Position, block4Velocity, block4Size, false);
    
    //Block5 setup (second 3 vertical tiles)
    Vector3 block5Position(16.5f, -8.5f, 0.0f);
    Vector3 block5Velocity(0.0f, 0.0f, 0.0f);
    Vector3 block5Size(1.0f, 3.0f, 0.0f);
    Entity* block5 = new Entity(block5Position, block5Velocity, block5Size, false);
    
    //Platform3 setup
    Vector3 plat3Position(22.0f, -9.5f, 0.0f);
    Vector3 plat3Velocity(0.0f, 0.0f, 0.0f);
    Vector3 plat3Size(8.0f, 1.0f, 0.0f);
    Entity* platform3 = new Entity(plat3Position, plat3Velocity, plat3Size, false);
    
    //viewMatrix.Translate(-player->position.x, -player->position.y, 0.0);
    viewMatrix.Translate(-10.0, 5.0, 0.0);
    
//To view last platform out of the screen
    //viewMatrix.Translate(-19.0, 5.0, 0.0);
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }else if(event.type == SDL_KEYDOWN) {
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                    moveSPACE = true;
                    Mix_PlayChannel(-1, jumpSound, 0);
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
                    moveDOWN = true;
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
                    moveLEFT = true;
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
                    moveRIGHT = true;
                }
            }
        }
        //Loop
        glClear(GL_COLOR_BUFFER_BIT);
    
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        
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
        
    //World objects stup
        program.setModelMatrix(modelWorld);
        
        //Floor
        //First platform
        float floortile1[] = {1.0, -10.0, 2.0, -9.0, 1.0, -9.0, 2.0, -9.0, 1.0, -10.0, 2.0, -10.0};
        DrawSpriteSheetSprite(&program, spritesheet, floortile1, 0, 16, 8);
        float floortile2[] = {2.0, -10.0, 3.0, -9.0, 2.0, -9.0, 3.0, -9.0, 2.0, -10.0, 3.0, -10.0};
        DrawSpriteSheetSprite(&program, spritesheet, floortile2, 17, 16, 8);
        float floortile3[] = {3.0, -10.0, 4.0, -9.0, 3.0, -9.0, 4.0, -9.0, 3.0, -10.0, 4.0, -10.0};
        DrawSpriteSheetSprite(&program, spritesheet, floortile3, 18, 16, 8);
        float floortile4[] = {4.0, -10.0, 5.0, -9.0, 4.0, -9.0, 5.0, -9.0, 4.0, -10.0, 5.0, -10.0};
        DrawSpriteSheetSprite(&program, spritesheet, floortile4, 2, 16, 8);
        
        //Second platform
        float floortile5[] = {9.0, -10.0, 10.0, -9.0, 9.0, -9.0, 10.0, -9.0, 9.0, -10.0, 10.0, -10.0};
        DrawSpriteSheetSprite(&program, spritesheet, floortile5, 3, 16, 8);
        float floortile6[] = {10.0, -10.0, 11.0, -9.0, 10.0, -9.0, 11.0, -9.0, 10.0, -10.0, 11.0, -10.0};
        DrawSpriteSheetSprite(&program, spritesheet, floortile6, 3, 16, 8);
        float floortile7[] = {11.0, -10.0, 12.0, -9.0, 11.0, -9.0, 12.0, -9.0, 11.0, -10.0, 12.0, -10.0};
        DrawSpriteSheetSprite(&program, spritesheet, floortile7, 3, 16, 8);
        float floortile8[] = {12.0, -10.0, 13.0, -9.0, 12.0, -9.0, 13.0, -9.0, 12.0, -10.0, 13.0, -10.0};
        DrawSpriteSheetSprite(&program, spritesheet, floortile8, 3, 16, 8);
        float floortile9[] = {13.0, -10.0, 14.0, -9.0, 13.0, -9.0, 14.0, -9.0, 13.0, -10.0, 14.0, -10.0};
        DrawSpriteSheetSprite(&program, spritesheet, floortile9, 3, 16, 8);
        float floortile10[] = {14.0, -10.0, 15.0, -9.0, 14.0, -9.0, 15.0, -9.0, 14.0, -10.0, 15.0, -10.0};
        DrawSpriteSheetSprite(&program, spritesheet, floortile10, 3, 16, 8);
        
        //Third platform
        float floortile11[] = {18.0, -10.0, 19.0, -9.0, 18.0, -9.0, 19.0, -9.0, 18.0, -10.0, 19.0, -10.0};
        DrawSpriteSheetSprite(&program, spritesheet, floortile11, 0, 16, 8);
        float floortile12[] = {19.0, -10.0, 20.0, -9.0, 19.0, -9.0, 20.0, -9.0, 19.0, -10.0, 20.0, -10.0};
        DrawSpriteSheetSprite(&program, spritesheet, floortile12, 1, 16, 8);
        float floortile13[] = {20.0, -10.0, 21.0, -9.0, 20.0, -9.0, 21.0, -9.0, 20.0, -10.0, 21.0, -10.0};
        DrawSpriteSheetSprite(&program, spritesheet, floortile13, 16, 16, 8);
        float floortile14[] = {21.0, -10.0, 22.0, -9.0, 21.0, -9.0, 22.0, -9.0, 21.0, -10.0, 22.0, -10.0};
        DrawSpriteSheetSprite(&program, spritesheet, floortile14, 17, 16, 8);
        float floortile15[] = {22.0, -10.0, 23.0, -9.0, 22.0, -9.0, 23.0, -9.0, 22.0, -10.0, 23.0, -10.0};
        DrawSpriteSheetSprite(&program, spritesheet, floortile15, 18, 16, 8);
        float floortile16[] = {23.0, -10.0, 24.0, -9.0, 23.0, -9.0, 24.0, -9.0, 23.0, -10.0, 24.0, -10.0};
        DrawSpriteSheetSprite(&program, spritesheet, floortile16, 2, 16, 8);
        float floortile17[] = {24.0, -10.0, 25.0, -9.0, 24.0, -9.0, 25.0, -9.0, 24.0, -10.0, 25.0, -10.0};
        DrawSpriteSheetSprite(&program, spritesheet, floortile17, 17, 16, 8);
        float floortile18[] = {25.0, -10.0, 26.0, -9.0, 25.0, -9.0, 26.0, -9.0, 25.0, -10.0, 26.0, -10.0};
        DrawSpriteSheetSprite(&program, spritesheet, floortile18, 2, 16, 8);
        
        
        //Blocks
        //Single block tile on platform1
        float blocktile1[] = {4.0, -9.0, 5.0, -8.0, 4.0, -8.0, 5.0, -8.0, 4.0, -9.0, 5.0, -9.0};
        DrawSpriteSheetSprite(&program, spritesheet, blocktile1, 3, 16, 8);
        
        //First vertitude block tile(3)
        float blocktile2[] = {7.0, -8.0, 8.0, -7.0, 7.0, -7.0, 8.0, -7.0, 7.0, -8.0, 8.0, -8.0};
        DrawSpriteSheetSprite(&program, spritesheet, blocktile2, 3, 16, 8);
        float blocktile3[] = {7.0, -9.0, 8.0, -8.0, 7.0, -8.0, 8.0, -8.0, 7.0, -9.0, 8.0, -9.0};
        DrawSpriteSheetSprite(&program, spritesheet, blocktile3, 3, 16, 8);
        float blocktile4[] = {7.0, -10.0, 8.0, -9.0, 7.0, -9.0, 8.0, -9.0, 7.0, -10.0, 8.0, -10.0};
        DrawSpriteSheetSprite(&program, spritesheet, blocktile4, 3, 16, 8);
        
        //Second vertical block tile(2)
        float blocktile5[] = {8.0, -9.0, 9.0, -8.0, 8.0, -8.0, 9.0, -8.0, 8.0, -9.0, 9.0, -9.0};
        DrawSpriteSheetSprite(&program, spritesheet, blocktile5, 3, 16, 8);
        float blocktile6[] = {8.0, -10.0, 9.0, -9.0, 8.0, -9.0, 9.0, -9.0, 8.0, -10.0, 9.0, -10.0};
        DrawSpriteSheetSprite(&program, spritesheet, blocktile6, 3, 16, 8);
        
        //Third vertical block tile(2)
        float blocktile7[] = {15.0, -10.0, 16.0, -9.0, 15.0, -9.0, 16.0, -9.0, 15.0, -10.0, 16.0, -10.0};
        DrawSpriteSheetSprite(&program, spritesheet, blocktile7, 3, 16, 8);
        float blocktile8[] = {15.0, -9.0, 16.0, -8.0, 15.0, -8.0, 16.0, -8.0, 15.0, -9.0, 16.0, -9.0};
        DrawSpriteSheetSprite(&program, spritesheet, blocktile8, 3, 16, 8);
        
        //Fourth vertical block tile(3)
        float blocktile9[] = {16.0, -10.0, 17.0, -9.0, 16.0, -9.0, 17.0, -9.0, 16.0, -10.0, 17.0, -10.0};
        DrawSpriteSheetSprite(&program, spritesheet, blocktile9, 3, 16, 8);
        float blocktile10[] = {16.0, -9.0, 17.0, -8.0, 16.0, -8.0, 17.0, -8.0, 16.0, -9.0, 17.0, -9.0};
        DrawSpriteSheetSprite(&program, spritesheet, blocktile10, 3, 16, 8);
        float blocktile11[] = {16.0, -8.0, 17.0, -7.0, 16.0, -7.0, 17.0, -7.0, 16.0, -8.0, 17.0, -8.0};
        DrawSpriteSheetSprite(&program, spritesheet, blocktile11, 3, 16, 8);
        
        //Background objects
        float cloudtile1[] = {2.0, -6.0, 3.0, -5.0, 2.0, -5.0, 3.0, -5.0, 2.0, -6.0, 3.0, -6.0};
        DrawSpriteSheetSprite(&program, spritesheet, cloudtile1, 96, 16, 8);
        float cloudtile2[] = {3.0, -6.0, 4.0, -5.0, 3.0, -5.0, 4.0, -5.0, 3.0, -6.0, 4.0, -6.0};
        DrawSpriteSheetSprite(&program, spritesheet, cloudtile2, 97, 16, 8);
        float cloudtile3[] = {7.0, -3.0, 8.0, -2.0, 7.0, -2.0, 8.0, -2.0, 7.0, -3.0, 8.0, -3.0};
        DrawSpriteSheetSprite(&program, spritesheet, cloudtile3, 96, 16, 8);
        float cloudtile4[] = {8.0, -3.0, 9.0, -2.0, 8.0, -2.0, 9.0, -2.0, 8.0, -3.0, 9.0, -3.0};
        DrawSpriteSheetSprite(&program, spritesheet, cloudtile4, 97, 16, 8);
        float cloudtile5[] = {10.0, -4.0, 11.0, -3.0, 10.0, -3.0, 11.0, -3.0, 10.0, -4.0, 11.0, -4.0};
        DrawSpriteSheetSprite(&program, spritesheet, cloudtile5, 113, 16, 8);
        float cloudtile6[] = {11.0, -4.0, 12.0, -3.0, 11.0, -3.0, 12.0, -3.0, 11.0, -4.0, 12.0, -4.0};
        DrawSpriteSheetSprite(&program, spritesheet, cloudtile6, 114, 16, 8);
        
        float treetile1[] = {7.0, -7.0, 8.0, -6.0, 7.0, -6.0, 8.0, -6.0, 7.0, -7.0, 8.0, -7.0};
        DrawSpriteSheetSprite(&program, spritesheet, treetile1, 119, 16, 8);
        float treetile2[] = {7.0, -6.0, 8.0, -5.0, 7.0, -5.0, 8.0, -5.0, 7.0, -6.0, 8.0, -6.0};
        DrawSpriteSheetSprite(&program, spritesheet, treetile2, 103, 16, 8);
        
        float glasstile1[] = {13.0, -9.0, 14.0, -8.0, 13.0, -8.0, 14.0, -8.0, 13.0, -9.0, 14.0, -9.0};
        DrawSpriteSheetSprite(&program, spritesheet, glasstile1, 116, 16, 8);
        float glasstile2[] = {14.0, -9.0, 15.0, -8.0, 14.0, -8.0, 15.0, -8.0, 14.0, -9.0, 15.0, -9.0};
        DrawSpriteSheetSprite(&program, spritesheet, glasstile2, 117, 16, 8);
        
        float mush1[] = {21.0, -9.0, 22.0, -8.0, 21.0, -8.0, 22.0, -8.0, 21.0, -9.0, 22.0, -9.0};
        DrawSpriteSheetSprite(&program, spritesheet, mush1, 28, 16, 8);
        float mush2[] = {22.0, -9.0, 23.0, -8.0, 22.0, -8.0, 23.0, -8.0, 22.0, -9.0, 23.0, -9.0};
        DrawSpriteSheetSprite(&program, spritesheet, mush2, 29, 16, 8);
        float mush2head[] = {22.0, -8.0, 23.0, -7.0, 22.0, -7.0, 23.0, -7.0, 22.0, -8.0, 23.0, -8.0};
        DrawSpriteSheetSprite(&program, spritesheet, mush2head, 14, 16, 8);
        float mush3[] = {23.0, -9.0, 24.0, -8.0, 23.0, -8.0, 24.0, -8.0, 23.0, -9.0, 24.0, -9.0};
        DrawSpriteSheetSprite(&program, spritesheet, mush3, 30, 16, 8);
        float mush3head[] = {23.0, -8.0, 24.0, -7.0, 23.0, -7.0, 24.0, -7.0, 23.0, -8.0, 24.0, -8.0};
        DrawSpriteSheetSprite(&program, spritesheet, mush3head, 15, 16, 8);
        float mush4[] = {24.0, -9.0, 25.0, -8.0, 24.0, -8.0, 25.0, -8.0, 24.0, -9.0, 25.0, -9.0};
        DrawSpriteSheetSprite(&program, spritesheet, mush4, 31, 16, 8);
        float mush4head[] = {24.0, -8.0, 25.0, -7.0, 24.0, -7.0, 25.0, -7.0, 24.0, -8.0, 25.0, -8.0};
        DrawSpriteSheetSprite(&program, spritesheet, mush4head, 15, 16, 8);
        
        
    //Enemy setup
        program.setModelMatrix(modelEnemy);
        float enemy[] = {9.0, -9.0, 10.0, -8.0, 9.0, -8.0, 10.0, -8.0, 9.0, -9.0, 10.0, -9.0};
        DrawSpriteSheetSprite(&program, spritesheet, enemy, 81, 16, 8);
        
        //Simple enemy movement since X collision is not implemented correctly
        float enemyRight = enemy1->position.x + enemy1->size.x/2;
        float enemyLeft = enemy1->position.x + enemy1->size.x/2;
        if(enemyRight >= 9.0 && enemyRight <= 15.00) {
            enemy1->velocity.x += enemyaccelerationX * FIXED_TIMESTEP;
            modelEnemy.Translate(enemy1->velocity.x, 0.0, 0.0);
            enemy1->position.x += enemy1->velocity.x;
        }
        
    //Player setup
        program.setModelMatrix(modelMatrix);
        
        if(moveSPACE == true) {
            accelerationY = 14.0;
            player->velocity.y += accelerationY * FIXED_TIMESTEP;
        }else if(moveDOWN == true) {
            accelerationY = 3.0f;
            player->velocity.y -= accelerationY * FIXED_TIMESTEP;
        }else if(moveLEFT == true) {
            player->velocity.x -= accelerationX * FIXED_TIMESTEP;
        }else if(moveRIGHT == true) {
            player->velocity.x += accelerationX * FIXED_TIMESTEP;
        }
        
        //Gravity
        modelMatrix.Translate(0.0, -gravity * FIXED_TIMESTEP, 0.0);
        player->position.y -= gravity * FIXED_TIMESTEP;
        
        player->velocity.y = lerp(player->velocity.y, 0.0f, FIXED_TIMESTEP * friction);
        player->velocity.x = lerp(player->velocity.x, 0.0f, FIXED_TIMESTEP * friction);
        
        modelMatrix.Translate(player->velocity.x, player->velocity.y, 0.0);
        moveSPACE = false;
        moveDOWN = false;
        moveLEFT = false;
        moveRIGHT = false;
        
        
        player->position.y += player->velocity.y;
        if(!collision(*platform1, *player, true, false)) {
            modelMatrix.Translate(0.0, adjustY + 0.01, 0.0);
            player->position.y += adjustY + 0.01;
        }
        if(!collision(*block1, *player, true, false)) {
            modelMatrix.Translate(0.0, adjustY + 0.01, 0.0);
            player->position.y += adjustY + 0.01;
        }
        if(!collision(*block2, *player, true, false)) {
            modelMatrix.Translate(0.0, adjustY + 0.01, 0.0);
            player->position.y += adjustY + 0.01;
        }
        if(!collision(*block3, *player, true, false)) {
            modelMatrix.Translate(0.0, adjustY + 0.01, 0.0);
            player->position.y += adjustY + 0.01;
        }
        if(!collision(*platform2, *player, true, false)) {
            modelMatrix.Translate(0.0, adjustY + 0.01, 0.0);
            player->position.y += adjustY + 0.01;
        }
        if(!collision(*block4, *player, true, false)) {
            modelMatrix.Translate(0.0, adjustY + 0.01, 0.0);
            player->position.y += adjustY + 0.01;
        }
        if(!collision(*block5, *player, true, false)) {
            modelMatrix.Translate(0.0, adjustY + 0.01, 0.0);
            player->position.y += adjustY + 0.01;
        }
        if(!collision(*platform3, *player, true, false)) {
            modelMatrix.Translate(0.0, adjustY + 0.01, 0.0);
            player->position.y += adjustY + 0.01;
        }

        if(!collision(*enemy1, *player, true, false)) {
            std::cout << "Game Over\n";
        }
        
        //X collision not working, player can go through walls
        player->position.x += player->velocity.x;
        if(!collision(*platform1, *player, false, true)) {
            modelMatrix.Translate(-adjustX + 0.01, 0.0, 0.0);
            player->position.x -= adjustX + 0.01;
        }
        
        //std::cout << "X: " << player->position.x << " Y: " << player->position.y << std::endl;
        
        float playerBody[] = {1.0, -6.0, 2.0, -5.0, 1.0, -5.0, 2.0, -5.0, 1.0, -6.0, 2.0, -6.0};
        float playerHead[] = {1.0, -5.0, 2.0, -4.0, 1.0, -4.0, 2.0, -4.0, 1.0, -5.0, 2.0, -5.0};
        DrawSpriteSheetSprite(&program, spritesheet, playerBody, 99, 16, 8);
        DrawSpriteSheetSprite(&program, spritesheet, playerHead, 83, 16, 8);
        SDL_GL_SwapWindow(displayWindow);
    }
    
    delete player;
    delete enemy1;
    delete platform1;
    delete platform2;
    delete platform3;
    delete block1;
    delete block2;
    delete block3;
    delete block4;
    delete block5;
    
    SDL_Quit();
    return 0;
}
