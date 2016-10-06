#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define PI 3.1415926536
#define TORADIANS (PI/180.0)

/*
 Had trouble making the ball bounce based on angle. Was unable to input the correct ball bounce angles.
 The other elements of the game, including y-axis and paddle collision are funcional and winner/loser is 
 decided based on whether ballX is passed paddleX and hits side border. Paddles can move up or down. 
 Starting ball movement was made random between 4 different cases.
 */

float player1X = -3.2f, player1Y = 0.0f;
float player2X = 3.2f, player2Y = 0.0f;
float player1vel = 3.0f, player2vel = 3.0f;
float playerwidth = 0.4f, playerheight = 1.5f;
float ballX = 0.0f, ballY = 0.0f;
float ballXvel = 2.0f, ballYvel = 2.0f;
float ballwidth = 0.5f, ballheight = 0.5f;
bool bounceoff1 = false, bounceoff2 = false, hittop = false, hitbot = false, player1win = false, player2win = false, player1side = false, player2side = false, bounceoff1pad = false, bounceoff2pad = false;

//Collision function to detect if ball collides with paddles
bool collision(float ballX, float ballY, float ballwidth, float ballheight, float playerX, float playerY, float playerwidth, float playerheight){
    if(ballY - ballheight/2 >= playerY + playerheight/2)   return true;
    if(ballY + ballheight/2 <= playerY - playerheight/2)   return true;
    if(ballX - ballwidth/2 >= playerX + playerwidth/2) return true;
    if(ballX + ballwidth/2 <= playerX - playerwidth/2) return true;
    return false;
}

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

SDL_Window* displayWindow;

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    
    //Setup
    ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

    Matrix projectionMatrix;
    Matrix modelMatrixPlayer1;
    Matrix modelMatrixPlayer2;
    Matrix modelMatrixBall;
    Matrix viewMatrix;
    
    projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glUseProgram(program.programID);
    
    GLuint playerTexture = LoadTexture("elementGlass037.png");
    GLuint ballTexture = LoadTexture("elementGlass005.png");
    
    float lastFrameTicks = 0.0f;
    
    //Random number generator in a ragne from 1 to 4
    srand(time(NULL));
    float randomnumber = rand() % 4 + 1;
    
    //Test cases
    //float randomnumber = 1;
    //float randomnumber = 2;
    //float randomnumber = 3;
    //float randomnumber = 4;
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }

        }
        
        //Loop
        glClear(GL_COLOR_BUFFER_BIT);
        
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        program.setModelMatrix(modelMatrixPlayer1);
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        
        float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0};

    //Player1 setup
        
        glBindTexture(GL_TEXTURE_2D, playerTexture);
        
        if(keys[SDL_SCANCODE_W]) {
            if(player1Y + playerheight/2 >= 1.95){
                //Do nothing
            }
            else {
                player1Y += player1vel * elapsed;
                modelMatrixPlayer1.Translate(0.0, player1vel * elapsed, 0.0);
            }
        }else if(keys[SDL_SCANCODE_S]) {
            if(player1Y - playerheight/2 <= -1.95){
                //Do nothing
            }
            else {
                player1Y -= player2vel * elapsed;
                modelMatrixPlayer1.Translate(0.0, -player1vel * elapsed, 0.0);
            }
        }
        
        float verticesPlayer1[] = {-3.4, -0.75, -3.0, -0.75, -3.0, 0.75, -3.4, -0.75, -3.0, 0.75, -3.4, 0.75};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticesPlayer1);
        glEnableVertexAttribArray(program.positionAttribute);
        
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
    //player2 setup
    
        program.setModelMatrix(modelMatrixPlayer2);
        glBindTexture(GL_TEXTURE_2D, playerTexture);
        
        if(keys[SDL_SCANCODE_UP]) {
            if(player2Y + playerheight/2 >= 1.95){
                //Do nothing
            }
            else {
                player2Y += player2vel * elapsed;
                modelMatrixPlayer2.Translate(0.0, player2vel * elapsed, 0.0);
            }
        }else if(keys[SDL_SCANCODE_DOWN]) {
            if(player2Y - playerheight/2 <= -1.95){
                //Do nothing
            }
            else {
                player2Y -= player2vel * elapsed;
                modelMatrixPlayer2.Translate(0.0, -player2vel * elapsed, 0.0);
            }
        }
        
        float verticesPlayer2[] = {3.0, -0.75, 3.4, -0.75, 3.4, 0.75, 3.0, -0.75, 3.4, 0.75, 3.0, 0.75};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticesPlayer2);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);

    //Ball setup
        program.setModelMatrix(modelMatrixBall);
        glBindTexture(GL_TEXTURE_2D, ballTexture);
        
        //Start ball movment is random based on RNG value
        std::cout << randomnumber << std::endl;
        if(!bounceoff1 && !bounceoff2 && !hittop && !hitbot && ticks > 1.0) {
            if(randomnumber == 1) {
                ballX += ballXvel * elapsed;
                ballY += ballYvel * elapsed;
                modelMatrixBall.Translate(ballXvel * elapsed , ballYvel * elapsed, 0.0);
            }
            else if(randomnumber == 2){
                ballX -= ballXvel * elapsed;
                ballY += ballYvel * elapsed;
                modelMatrixBall.Translate(-ballXvel * elapsed , ballYvel * elapsed, 0.0);
            }
            else if(randomnumber == 3) {
                ballX += ballXvel * elapsed;
                ballY -= ballYvel * elapsed;
                modelMatrixBall.Translate(ballXvel * elapsed , -ballYvel * elapsed, 0.0);
            }
            else if(randomnumber == 4) {
                ballX -= ballXvel * elapsed;
                ballY -= ballYvel * elapsed;
                modelMatrixBall.Translate(-ballXvel * elapsed , -ballYvel * elapsed, 0.0);
            }
        }
        
        //Paddle collision if statement
        if(!collision(ballX, ballY, ballwidth, ballheight, player1X, player1Y, playerwidth, playerheight)) {
            bounceoff2 = false;
            bounceoff1 = true;
            hitbot = false;
            hittop = false;
            player1side = false;
            player2side = false;
        }
        else if(!collision(ballX, ballY, ballwidth, ballheight, player2X, player2Y, playerwidth, playerheight)) {
            bounceoff2 = true;
            bounceoff1 = false;
            hitbot = false;
            hittop = false;
            player1side = false;
            player2side = false;
        }
        //Player 1 win condition
        else if(ballX + ballwidth/2 >= 3.55) {
            player1win = true;
        }
        //Player 2 win condition
        else if(ballX - ballwidth/2 <= -3.55) {
            player2win = true;
        }
        //Ball Y hits bot on player 1 side
        else if(ballY - ballheight <= -2.25 && ballX - ballwidth < 0) {
            hitbot = true;
            hittop = false;
            player1side = true;
            player2side = false;
        }
        //BallY hits bot on player 2 side
        else if(ballY - ballheight <= -2.25 && ballX + ballwidth > 0) {
            hitbot = true;
            hittop = false;
            player1side = false;
            player2side = true;
        }
        //BallY hits top on player 1 side without paddle collision
        else if(ballY + ballheight >= 2.25 && ballX - ballwidth < 0 && !collision(ballX, ballY, ballwidth, ballheight, player1X, player1Y, playerwidth, playerheight)) {
            hitbot = false;
            hittop = true;
            player1side = true;
            player2side = false;
            bounceoff1pad = false;
        }
        //BallY hits top on player 1 side with paddle collision
        else if(ballY + ballheight >= 2.25 && ballX - ballwidth < 0 && collision(ballX, ballY, ballwidth, ballheight, player1X, player1Y, playerwidth, playerheight)) {
            hitbot = false;
            hittop = true;
            player1side = true;
            player2side = false;
            bounceoff1pad = true;
        }
        //BallY hits top on player 2 side without paddle collision
        else if(ballY + ballheight >= 2.25 && ballX + ballwidth > 0 && !collision(ballX, ballY, ballwidth, ballheight, player2X, player2Y, playerwidth, playerheight)) {
            hitbot = false;
            hittop = true;
            player1side = false;
            player2side = true;
            bounceoff2pad = false;
        }
        //BallY hits top on player 2 side with paddle collsiion
        else if(ballY + ballheight >= 2.25 && ballX + ballwidth > 0 && collision(ballX, ballY, ballwidth, ballheight, player2X, player2Y, playerwidth, playerheight)) {
            hitbot = false;
            hittop = true;
            player1side = false;
            player2side = true;
            bounceoff2pad = true;
        }
        
        /*
         All possible scenarios based of true/false conditions.
         Was not able to get angles to work correctly.
         Game starts with wrong first bounce off top wall and does a couple of correct bounces before being
         stuck with a repeated bounce.
        */
        
        if(player1win) {
            modelMatrixBall.identity();
            modelMatrixPlayer1.identity();
            modelMatrixPlayer2.identity();
            std::cout << "Player 1 WIN!\n";
        }
        else if(player2win) {
            modelMatrixBall.identity();
            modelMatrixPlayer1.identity();
            modelMatrixPlayer2.identity();
            std::cout << "Player 2 WIN!\n";
        }
        //-----------Attempted if statesments for angle based on hit of paddle--------
        //Ball hits top on player 1 side without paddle collision
        else if(hittop && !hitbot && player1side && !player2side && !bounceoff1pad) {
            ballX -= ballXvel * elapsed;
            ballY -= ballYvel * elapsed;
            modelMatrixBall.Translate(-ballXvel * elapsed, -ballYvel * elapsed, 0.0);
        }
        //Ball hits top on player 1 side without paddle collision
        else if(hittop && !hitbot && player1side && !player2side && bounceoff1pad) {
            ballX += ballXvel * elapsed;
            ballY -= ballYvel * elapsed;
            modelMatrixBall.Translate(ballXvel * elapsed, -ballYvel * elapsed, 0.0);
        }
        //Ball hits top on player 2 side without paddle collision
        else if(hittop && !hitbot && !player1side && player2side && !bounceoff2pad) {
            ballX += ballXvel * elapsed;
            ballY -= ballYvel * elapsed;
            modelMatrixBall.Translate(ballXvel * elapsed, -ballYvel * elapsed, 0.0);
        }
        //Ball hits top on player 2 side with paddle collision
        else if(hittop && !hitbot && !player1side && player2side && bounceoff2pad) {
            ballX -= ballXvel * elapsed;
            ballY -= ballYvel * elapsed;
            modelMatrixBall.Translate(-ballXvel * elapsed, -ballYvel * elapsed, 0.0);
        }
        //-------------If statement below do not account for angle hit off of paddle-----
        //Ball hits bottom on player 1 side
        else if(hitbot && !hittop && player1side && !player2side) {
            ballX -= ballXvel * elapsed;
            ballY += ballYvel * elapsed;
            modelMatrixBall.Translate(-ballXvel * elapsed, ballYvel * elapsed, 0.0);
        }
        //Ball hits bottom on player 2 side
        else if(hitbot && !hittop && !player1side && player2side) {
            ballX += ballXvel * elapsed;
            ballY += ballYvel * elapsed;
            modelMatrixBall.Translate(ballXvel * elapsed, ballYvel * elapsed, 0.0);
        }
        //Ball hits player 1 paddle from bot
        else if(bounceoff1 && !bounceoff2){
            ballX += ballXvel * elapsed;
            ballY += ballYvel * elapsed;
            modelMatrixBall.Translate(ballXvel * elapsed, ballYvel * elapsed, 0.0);
        }
        //Ball hits player 2 paddle from bot
        else if(bounceoff2 && !bounceoff1){
            ballX -= ballXvel * elapsed;
            ballY += ballYvel * elapsed;
            modelMatrixBall.Translate(-ballXvel * elapsed, ballYvel * elapsed, 0.0);
        }

        
        float verticesBall[] = {-0.25, -0.25, 0.25, -0.25, 0.25, 0.25, -0.25, -0.25, 0.25, 0.25, -0.25, 0.25};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticesBall);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
