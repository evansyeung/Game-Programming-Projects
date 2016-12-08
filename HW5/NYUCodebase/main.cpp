/*
    Evans Yeung
    SAT Collision
    
    Rotation and scale were not implemented.
    Collision only works for when the bottom of the ship collides with the top of the meteors.
*/

#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include <vector>

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define PI 3.1415926536
#define TORADIANS (PI/180.0)
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

float lastFrameTicks = 0.0f;
float friction = 2.0;
float accelerationX = 3.0f, accelerationY = 3.0f;
float tempVelX = 0.0f, tempVelY = 0.0f;
float bmAccelerationX = 1.0f, bmAccelerationY = 1.0f;
bool moveUp = false, moveDown = false, moveLeft = false, moveRight = false;
bool bmTopCollide = false, bmBotCollide = false, bmLeftCollide = false, bmRightCollide = false;
bool smTopCollide = false, smBotCollide = false, smLeftCollide = false, smRightCollide = false;
const Uint8 *keys = SDL_GetKeyboardState(NULL);
float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0};

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

class Vector3 {
public:
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    
    float x;
    float y;
    float z;
};

class Entity {
public:
    Entity(Vector3 position, Vector3 velocity, Vector3 size, Vector3 topLeft, Vector3 topRight, Vector3 botLeft, Vector3 botRight) : position(position), velocity(velocity), size(size), topLeft(topLeft), topRight(topRight), botLeft(botLeft), botRight(botRight) {}
    
    Vector3 position;
    Vector3 velocity;
    Vector3 size;
    Vector3 topLeft;
    Vector3 topRight;
    Vector3 botLeft;
    Vector3 botRight;
};

bool testSATSeparationForEdge(float edgeX, float edgeY, const std::vector<Vector3> &points1, const std::vector<Vector3> &points2) {
    float normalX = -edgeY;
    float normalY = edgeX;
    float len = sqrtf(normalX*normalX + normalY*normalY);
    normalX /= len;
    normalY /= len;
    
    std::vector<float> e1Projected;
    std::vector<float> e2Projected;
    
    for(int i=0; i < points1.size(); i++) {
        e1Projected.push_back(points1[i].x * normalX + points1[i].y * normalY);
    }
    for(int i=0; i < points2.size(); i++) {
        e2Projected.push_back(points2[i].x * normalX + points2[i].y * normalY);
    }
    
    std::sort(e1Projected.begin(), e1Projected.end());
    std::sort(e2Projected.begin(), e2Projected.end());
    
    float e1Min = e1Projected[0];
    float e1Max = e1Projected[e1Projected.size()-1];
    float e2Min = e2Projected[0];
    float e2Max = e2Projected[e2Projected.size()-1];
    float e1Width = fabs(e1Max-e1Min);
    float e2Width = fabs(e2Max-e2Min);
    float e1Center = e1Min + (e1Width/2.0);
    float e2Center = e2Min + (e2Width/2.0);
    float dist = fabs(e1Center-e2Center);
    float p = dist - ((e1Width+e2Width)/2.0);
    
    if(p < 0) {
        return true;
    }
    return false;
}

bool checkSATCollision(const std::vector<Vector3> &e1Points, const std::vector<Vector3> &e2Points) {
    for(int i=0; i < e1Points.size(); i++) {
        float edgeX, edgeY;
        
        if(i == e1Points.size()-1) {
            edgeX = e1Points[0].x - e1Points[i].x;
            edgeY = e1Points[0].y - e1Points[i].y;
        } else {
            edgeX = e1Points[i+1].x - e1Points[i].x;
            edgeY = e1Points[i+1].y - e1Points[i].y;
        }
        
        bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points);
        if(!result) {
            return false;
        }
    }
    for(int i=0; i < e2Points.size(); i++) {
        float edgeX, edgeY;
        
        if(i == e2Points.size()-1) {
            edgeX = e2Points[0].x - e2Points[i].x;
            edgeY = e2Points[0].y - e2Points[i].y;
        } else {
            edgeX = e2Points[i+1].x - e2Points[i].x;
            edgeY = e2Points[i+1].y - e2Points[i].y;
        }
        bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points);
        if(!result) {
            return false;
        }
    }
    return true;
}

float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}

SDL_Window* displayWindow;

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif

    ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    Matrix projectionMatrix;
    Matrix modelMatrix;
    Matrix viewMatrix;
    Matrix modelBigMeteor;
    Matrix modelSmallMeteor;

    projectionMatrix.setOrthoProjection(-8.88, 8.88, -5.0f, 5.0f, -1.0f, 1.0f);
    glUseProgram(program.programID);
    
    GLuint shipTexture = LoadTexture("playerShip2_orange.png");
    GLuint bigMeteorTexture = LoadTexture("meteorBrown_big3.png");
    GLuint smallMeteorTexture = LoadTexture("meteorBrown_small1.png");
    
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
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Vector3 shipPosition(0.0f, 0.0f, 0.0f);
    Vector3 shipVelocity(0.0f, 0.0f, 0.0f);
    Vector3 shipSize(1.0f, 1.0f, 0.0f);
    Vector3 shipTL(-0.5, 0.5, 0.0);
    Vector3 shipTR(0.5, -0.5, 0.0);
    Vector3 shipBL(-0.5, -0.5, 0.0);
    Vector3 shipBR(0.5, -0.5, 0.0);
    Entity* ship = new Entity(shipPosition, shipVelocity, shipSize, shipTL, shipTR, shipBL, shipBR);
    
    Vector3 bMeteorPosition(2.0f, 2.0f, 0.0f);
    Vector3 bMeteorVelocity(0.0f, 0.0f, 0.0f);
    Vector3 bMeteorSize(2.0f, 2.0f, 0.0f);
    Vector3 bMetTL(1.0, 3.0, 0.0);
    Vector3 bMetTR(3.0, 3.0, 0.0);
    Vector3 bMetBL(1.0, 1.0, 0.0);
    Vector3 bMetBR(3.0, 1.0, 0.0);
    Entity* bMeteor = new Entity(bMeteorPosition, bMeteorVelocity, bMeteorSize, bMetTL, bMetTR, bMetBL, bMetBR);
    
    Vector3 sMeteorPosition(-1.75f, -2.25f, 0.0f);
    Vector3 sMeteorVelocity(0.0f, 0.0f, 0.0f);
    Vector3 sMeteorSize(0.5f, 0.5f, 0.0f);
    Vector3 sMetTL(-2.0, -2.0, 0.0);
    Vector3 sMetTR(-1.5, -2.0, 0.0);
    Vector3 sMetBL(-2.0, -2.5, 0.0);
    Vector3 sMetBR(-1.5, -2.5, 0.0);
    Entity* sMeteor = new Entity(sMeteorPosition, sMeteorVelocity, sMeteorSize, sMetTL, sMetTR, sMetBL, sMetBR);
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }else if(event.type == SDL_KEYDOWN) {
                if(event.key.keysym.scancode == SDL_SCANCODE_UP) {
                    moveUp = true;
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
        glClearColor(1.0, 0.0, 1.0, 1.0);
        
        glClear(GL_COLOR_BUFFER_BIT);

        program.setModelMatrix(modelMatrix);
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        
        glBindTexture(GL_TEXTURE_2D, shipTexture);
        
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

        float verticesShip[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticesShip);
        glEnableVertexAttribArray(program.positionAttribute);
        
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        program.setModelMatrix(modelBigMeteor);
        glBindTexture(GL_TEXTURE_2D, bigMeteorTexture);
        
        std::vector<Vector3> shipTop;
        shipTop.push_back(ship->topLeft);
        shipTop.push_back(ship->topRight);
        
        std::vector<Vector3> bMeteorTop;
        bMeteorTop.push_back(bMeteor->topLeft);
        bMeteorTop.push_back(bMeteor->topRight);
        
        std::vector<Vector3> sMeteorTop;
        sMeteorTop.push_back(sMeteor->topLeft);
        sMeteorTop.push_back(sMeteor->topRight);
        
        std::vector<Vector3> shipBot;
        shipBot.push_back(ship->botLeft);
        shipBot.push_back(ship->botRight);
        
        std::vector<Vector3> shipLeft;
        shipLeft.push_back(ship->topLeft);
        shipLeft.push_back(ship->botLeft);
        
        std::vector<Vector3> shipRight;
        shipRight.push_back(ship->topRight);
        shipRight.push_back(ship->botRight);
        
        std::vector<Vector3> bMeteorBot;
        bMeteorBot.push_back(bMeteor->botLeft);
        bMeteorBot.push_back(bMeteor->botRight);
        
        std::vector<Vector3> bMeteorRight;
        bMeteorRight.push_back(bMeteor->topLeft);
        bMeteorRight.push_back(bMeteor->botLeft);
        
        std::vector<Vector3> bMeteorLeft;
        bMeteorLeft.push_back(bMeteor->topLeft);
        bMeteorLeft.push_back(bMeteor->botLeft);

        
        if(moveUp) {
            ship->velocity.y += accelerationY * FIXED_TIMESTEP;
        }else if(moveDown) {
            ship->velocity.y -= accelerationY * FIXED_TIMESTEP;
        }else if(moveLeft) {
            ship->velocity.x -= accelerationX * FIXED_TIMESTEP;
        }else if(moveRight) {
            ship->velocity.x += accelerationX * FIXED_TIMESTEP;
        }
        
        ship->velocity.y = lerp(ship->velocity.y, 0.0f, FIXED_TIMESTEP * friction);
        ship->velocity.x = lerp(ship->velocity.x, 0.0f, FIXED_TIMESTEP * friction);
        
        modelMatrix.Translate(ship->velocity.x, ship->velocity.y, 0.0);
        moveUp = false;
        moveDown = false;
        moveLeft = false;
        moveRight = false;
        
        //Update vertices
        ship->topLeft.x += ship->velocity.x;
        ship->topRight.x += ship->velocity.x;
    
        
        ship->botLeft.x += ship->velocity.x;
        ship->botRight.x += ship->velocity.x;

        ship->topLeft.y += ship->velocity.y;
        ship->topRight.y += ship->velocity.y;
        
        ship->botLeft.y += ship->velocity.y;
        ship->botRight.y += ship->velocity.y;

        if(checkSATCollision(shipTop, bMeteorTop)) {
            std::cout << "Top Collision Big Meteor\n";
            bmTopCollide = true;
            tempVelX = ship->velocity.x;
            tempVelY = ship->velocity.y;
            ship->velocity.y = 0.0f;
            ship->velocity.x = 0.0f;
        }
        
        if(checkSATCollision(shipTop, sMeteorTop)) {
            std::cout << "Top Collision Small Meteor\n";
            smTopCollide = true;
            tempVelX = ship->velocity.x;
            tempVelY = ship->velocity.y;
            ship->velocity.y = 0.0f;
            ship->velocity.x = 0.0f;
        }
        
        if(checkSATCollision(shipBot, bMeteorBot)) {
            bmBotCollide = true;
            tempVelX = ship->velocity.x;
            tempVelY = ship->velocity.y;
            ship->velocity.y = 0.0f;
            ship->velocity.x = 0.0f;
        }
        
        if(checkSATCollision(shipLeft, bMeteorLeft)) {
            std::cout << "Left Collision Big Meteor\n";
        }
        
        std::vector<Vector3> sMeteorLeft;
        sMeteorLeft.push_back(sMeteor->topLeft);
        sMeteorLeft.push_back(bMeteor->topRight);
        
        if(checkSATCollision(shipLeft, sMeteorLeft)) {
            std::cout << "Left Collision Small Meteor\n";
        }
        
        //---------
        
        if(bmTopCollide) {
            bMeteor->velocity.x += 50*tempVelX * FIXED_TIMESTEP;
            bMeteor->velocity.y += 50*tempVelY * FIXED_TIMESTEP;
        }else if(bmBotCollide) {
            bMeteor->velocity.x += -50*tempVelX * FIXED_TIMESTEP;
            bMeteor->velocity.y += -50*tempVelY * FIXED_TIMESTEP;
        }
        
        bMeteor->velocity.y = lerp(bMeteor->velocity.y, 0.0f, FIXED_TIMESTEP * friction);
        bMeteor->velocity.x = lerp(bMeteor->velocity.x, 0.0f, FIXED_TIMESTEP * friction);
        
        modelBigMeteor.Translate(bMeteor->velocity.x, bMeteor->velocity.y, 0.0);
        bmTopCollide = false;
        
        bMeteor->topLeft.x += bMeteor->velocity.x;
        bMeteor->topRight.x += bMeteor->velocity.x;
        
        bMeteor->botLeft.x += bMeteor->velocity.x;
        bMeteor->botRight.x += bMeteor->velocity.x;
        
        bMeteor->topLeft.y += bMeteor->velocity.y;
        bMeteor->topRight.y += bMeteor->velocity.y;
        
        bMeteor->botLeft.y += bMeteor->velocity.y;
        bMeteor->botRight.y += bMeteor->velocity.y;
    
        
        float bigMeteor[] = {1.0, 1.0, 3.0, 1.0, 3.0, 3.0, 1.0, 1.0, 3.0, 3.0, 1.0, 3.0};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, bigMeteor);
        glEnableVertexAttribArray(program.positionAttribute);
        
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        //-------------------------------
        
        if(smTopCollide) {
            sMeteor->velocity.x += 50*tempVelX * FIXED_TIMESTEP;
            sMeteor->velocity.y += 50*tempVelY * FIXED_TIMESTEP;
        }
        
        sMeteor->velocity.y = lerp(sMeteor->velocity.y, 0.0f, FIXED_TIMESTEP * friction);
        sMeteor->velocity.x = lerp(sMeteor->velocity.x, 0.0f, FIXED_TIMESTEP * friction);
        
        modelSmallMeteor.Translate(sMeteor->velocity.x, sMeteor->velocity.y, 0.0);
        smTopCollide = false;
        
        sMeteor->topLeft.x += sMeteor->velocity.x;
        sMeteor->topRight.x += sMeteor->velocity.x;
        
        sMeteor->botLeft.x += sMeteor->velocity.x;
        sMeteor->botRight.x += sMeteor->velocity.x;
        
        sMeteor->topLeft.y += sMeteor->velocity.y;
        sMeteor->topRight.y += sMeteor->velocity.y;
        
        sMeteor->botLeft.y += sMeteor->velocity.y;
        sMeteor->botRight.y += sMeteor->velocity.y;
        
        program.setModelMatrix(modelSmallMeteor);
        glBindTexture(GL_TEXTURE_2D, smallMeteorTexture);
        
        float smallMeteor[] = {-2.0, -2.5, -1.5, -2.5, -1.5, -2.0, -2.0, -2.5, -1.5, -2.0, -2.0, -2.0};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, smallMeteor);
        glEnableVertexAttribArray(program.positionAttribute);
        
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        SDL_GL_SwapWindow(displayWindow);
        
    }
    
    delete ship;
    delete bMeteor;
    delete sMeteor;
    
    SDL_Quit();
    return 0;
}
