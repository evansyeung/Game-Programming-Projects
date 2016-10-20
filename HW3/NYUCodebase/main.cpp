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

/*
 Evans Yeung
 HW3 Space Invader
 
 There are 3 games states: STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_END. Pressing any key will switch the menu state to game level. The player can move up, down, left, right and shoot with space. Colliding with enemy invaders results in transitioning to the game end state. Each invader shot down will add 100 points to the score. If the score reachse 2100, then the player wil transition to the game end states.
 
 Problems: Had trouble implementing the bullets. There is only one bullet that the player can shoot and enemys do not shoot bullets. Sometimes if the bullets get past the first two row of enemies and hits the top enemy, all enemies in the column will disappear.
 */

float friction = 3.0;
int score = 0, bullet_header = 0;
bool playerShot = false, start1Moving = true, start2Moving = true, start3Moving = true, row1HitRight = false, row1HitLeft = false, row2HitRight = false, row2HitLeft = false, row3HitRight = false, row3HitLeft = false;

enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_END };

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

class SheetSprite {
public:
    SheetSprite();
    SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size) : textureID(textureID), u(u), v(v), width(width), height(height), size(size) {};
        void Draw(ShaderProgram *program, float polygonX, float polygonY, float n);
        float size;
        unsigned int textureID;
        float u;
        float v;
        float width;
        float height;
};

//Added extra parameter to change polygon vertices
void SheetSprite::Draw(ShaderProgram *program, float polygonX, float polygonY, float q) {
    glBindTexture(GL_TEXTURE_2D, textureID);
    GLfloat texCoords[] = {
        u, v+height,
        u+width, v,
        u, v,
        u+width, v,
        u, v+height,
        u+width, v+height
    };
    float aspect = width / height;
    
    //Changed to variables from 0.5 for all values
    float vertices[] = {
        -polygonX * size * aspect + q, -polygonY * size,
        polygonX * size * aspect + q, polygonY * size,
        -polygonX * size * aspect + q, polygonY * size,
        polygonX * size * aspect + q, polygonY * size,
        -polygonX * size * aspect + q, -polygonY * size ,
        polygonX * size * aspect + q, -polygonY * size};
    
    // draw our arrays
    
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
    Entity(Vector3 position, Vector3 velocity, Vector3 size, SheetSprite sprite, bool dead) : position(position), velocity(velocity), size(size), sprite(sprite), dead(NULL) {}
    
    Vector3 position;
    Vector3 velocity;
    Vector3 size;
    
    bool dead = false;
    
    SheetSprite sprite;
};

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

bool collision(Entity one, Entity two){
    if(one.position.y - one.size.y/2 >= two.position.y + two.size.y/2)   return true;
    if(one.position.y + one.size.y/2 <= two.position.y - two.size.y/2)   return true;
    if(one.position.x - one.size.x/2 >= two.position.x + two.size.x/2) return true;
    if(one.position.x + one.size.x/2 <= two.position.x - two.size.x/2) return true;
    return false;
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
    Matrix modelGameName;
    Matrix modelPressButton;
    Matrix modelPlayer;
    Matrix modelBullet;
    Matrix modelEnemyRow1;
    Matrix modelEnemyRow2;
    Matrix modelEnemyRow3;
    Matrix modelScore;
    Matrix viewMatrix;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    
    GLuint fontTexture = LoadTexture("font1.png");
    GLuint spriteSheetTexture = LoadTexture("sheet.png");
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glUseProgram(program.programID);
    
    float lastFrameTicks = 0.0f;
    
    //Player Setup
    Vector3 playerPosition(0.0f, 0.0f, 0.0f);
    Vector3 playerVelocity(2.5f, 2.5f, 0.0f);
    Vector3 playerSize(0.528f, 0.4f, 0.0f);
    SheetSprite playerSprite(spriteSheetTexture, 211.0f/1024.0f, 941.0f/1024.0f, 99.0f/1024.0f, 75.0f/1024.0f, 0.2);
    Entity* player = new Entity(playerPosition, playerVelocity, playerSize, playerSprite, false);
    
    //Bullet Setup
    //Was not sure how to array to hold bullets
    Vector3 bulletPosition(0.0f, 0.0f, 0.0f);
    Vector3 bulletVelocity(2.5f, 2.5f, 0.0);
    Vector3 bulletSize(0.0f, 0.0f, 0.0f);
    SheetSprite bulletSprite(spriteSheetTexture, 858.0f/1024.0f, 475.0f/1024.0f, 9.0f/1024.0f, 37.0f/1024.0f, 0.2);
    Entity* bullets[5];
    for(int i = 0; i < 5; i++) {
        Entity* bullet = new Entity(bulletPosition, bulletVelocity, bulletSize, bulletSprite, true);
        bullets[i] = bullet;
    }
    
    //Enemy Setup
    std::vector<Entity*> enemiesRow1;
    for(int i = 0; i < 7; i++) {
        Vector3 enemyPosition(0.0f, 0.0f, 0.0f);
        Vector3 enemyVelocity(0.5f, 0.0f, 0.0f);
        Vector3 enemySize(0.442f, 0.4f, 0.0f);
        SheetSprite enemySprite(spriteSheetTexture, 425.0f/1024.0f, 552.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 0.2);
        Entity* enemy = new Entity(enemyPosition, enemyVelocity, enemySize, enemySprite, false);
        enemiesRow1.push_back(enemy);
    }
    
    std::vector<Entity*> enemiesRow2;
    for(int i = 0; i < 7; i++) {
        Vector3 enemyPosition(0.0f, 0.0f, 0.0f);
        Vector3 enemyVelocity(0.5f, 0.0f, 0.0f);
        Vector3 enemySize(0.442f, 0.4f, 0.0f);
        SheetSprite enemySprite(spriteSheetTexture, 425.0f/1024.0f, 552.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 0.2);
        Entity* enemy = new Entity(enemyPosition, enemyVelocity, enemySize, enemySprite, false);
        enemiesRow2.push_back(enemy);
    }
    
    std::vector<Entity*> enemiesRow3;
    for(int i = 0; i < 7; i++) {
        Vector3 enemyPosition(0.0f, 0.0f, 0.0f);
        Vector3 enemyVelocity(0.5f, 0.5f, 0.0f);
        Vector3 enemySize(0.442f, 0.4f, 0.0f);
        SheetSprite enemySprite(spriteSheetTexture, 425.0f/1024.0f, 552.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 0.2);
        Entity* enemy = new Entity(enemyPosition, enemyVelocity, enemySize, enemySprite, false);
        enemiesRow3.push_back(enemy);
    }
    
    modelGameName.Translate(-2.0, 1.0, 0.0);
    modelPressButton.Translate(-2.25, -0.05, 0.0);
    modelPlayer.Translate(0.0, -1.5, 0.0);
    modelScore.Translate(-3.45, -1.8, 0.0);
    modelEnemyRow1.Translate(-2.8, 1.7, 0.0);
    modelEnemyRow2.Translate(-2.8, 1.2, 0.0);
    modelEnemyRow3.Translate(-2.8, 0.7, 0.0);
    modelBullet.Translate(0.0, -2.0, 0.0);
    
    //Update (x,y) for each model translate above
    player->position.y += -1.5;
    
    for(int i = 0; i < 2; i++) {
        bullets[i]->position.y += -2.0;
    }

    enemiesRow1[0]->position.x += -2.8;
    enemiesRow1[1]->position.x += -1.8;
    enemiesRow1[2]->position.x += -0.8;
    enemiesRow1[3]->position.x += 0.2;
    enemiesRow1[4]->position.x += 1.2;
    enemiesRow1[5]->position.x += 2.2;
    enemiesRow1[6]->position.x += 3.2;
    for(int q = 0; q < enemiesRow1.size(); q++) {
        enemiesRow1[q]->position.y += 1.7;
    }
    
    enemiesRow2[0]->position.x += -2.8;
    enemiesRow2[1]->position.x += -1.8;
    enemiesRow2[2]->position.x += -0.8;
    enemiesRow2[3]->position.x += 0.2;
    enemiesRow2[4]->position.x += 1.2;
    enemiesRow2[5]->position.x += 2.2;
    enemiesRow2[6]->position.x += 3.2;
    for(int q = 0; q < enemiesRow2.size(); q++) {
        enemiesRow2[q]->position.y += 1.2;
    }
    
    enemiesRow3[0]->position.x += -2.8;
    enemiesRow3[1]->position.x += -1.8;
    enemiesRow3[2]->position.x += -0.8;
    enemiesRow3[3]->position.x += 0.2;
    enemiesRow3[4]->position.x += 1.2;
    enemiesRow3[5]->position.x += 2.2;
    enemiesRow3[6]->position.x += 3.2;
    for(int q = 0; q < enemiesRow3.size(); q++) {
        enemiesRow3[q]->position.y += 0.7;
    }
    
    int state = STATE_MAIN_MENU;
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }else if(event.type == SDL_KEYDOWN && state == STATE_MAIN_MENU) {
                state = STATE_GAME_LEVEL;
            }else if(event.type == SDL_KEYDOWN && state == STATE_GAME_LEVEL) {
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                    std::cout << "Shoot!\n";
                    playerShot = true;
                }
            }
        }
        
    //Loop
        glClear(GL_COLOR_BUFFER_BIT);
        
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        program.setProjectionMatrix(projectionMatrix);
        program.setViewMatrix(viewMatrix);
        
        
    //Game State switch case
        switch(state) {
            case STATE_MAIN_MENU:
                
                //Game Name Texture
                program.setModelMatrix(modelGameName);
                DrawText(&program, fontTexture, "Space Invader", 0.3, 0);
                
                //Press Button Texture
                program.setModelMatrix(modelPressButton);
                DrawText(&program, fontTexture, "Press any key to start!", 0.2, 0);
            break;
                
            case STATE_GAME_LEVEL:
                
            //Player model
                program.setModelMatrix(modelPlayer);
                player->sprite.Draw(&program, 1.0, 1.0, 0.0);
                std::cout << "Player x: " << player->position.x << " y: " << player->position.y << std::endl;
            
                //Player & Bullet movement
                if(keys[SDL_SCANCODE_UP]) {
                    if(player->position.y + player->size.y/2 >= 2.0) {
                        //Do nothing
                    }
                    else {
                        player->position.y += player->velocity.y * elapsed;
                        modelPlayer.Translate(0.0, player->velocity.y * elapsed, 0.0);;
                    }
                }else if(keys[SDL_SCANCODE_DOWN]) {
                    if(player->position.y - player->size.y/2 <= -1.65) {
                        //Do nothing
                    }
                    else {
                        player->position.y -= player->velocity.y * elapsed;
                        modelPlayer.Translate(0.0, -player->velocity.y * elapsed, 0.0);
                    }
                }else if(keys[SDL_SCANCODE_LEFT]) {
                    if(player->position.x - player->size.x/2 <= -3.5) {
                        //Do nothing
                    }
                    else {
                        player->position.x -= player->velocity.x * elapsed;
                        modelPlayer.Translate(-player->velocity.x * elapsed, 0.0, 0.0);
                    }
                }else if(keys[SDL_SCANCODE_RIGHT]) {
                    if(player->position.x + player->size.x/2 >= 3.5) {
                        //Do nothing
                    }
                    else {
                        player->position.x += player->velocity.x * elapsed;
                        modelPlayer.Translate(player->velocity.x * elapsed, 0.0, 0.0);

                    }
                }
            
            //Bullet Model
                program.setModelMatrix(modelBullet);
                
                std::cout << "Bullet x: " << bullets[bullet_header]->position.x << " y: " << bullets[bullet_header]->position.y << std::endl;
                
                //Uses the playerShot to check if the bullet has not been shot. Moves bullet to player's x position.
                if(bullets[bullet_header]->position.x != player->position.x && !playerShot){
                    if(bullets[bullet_header]->position.x <= player->position.x) {
                        bullets[bullet_header]->position.x += bullets[bullet_header]->velocity.x * elapsed;
                        modelBullet.Translate(bullets[bullet_header]->velocity.x * elapsed, 0.0, 0.0);
                    }else if(bullets[bullet_header]->position.x >= player->position.x && !bullets[bullet_header]->dead) {
                        bullets[bullet_header]->position.x -= bullets[bullet_header]->velocity.x * elapsed;
                        modelBullet.Translate(-bullets[bullet_header]->velocity.x * elapsed, 0.0, 0.0);
                    }
                }
                
                //Uses the dead flag to check if the bullet has not been shot. Moves bullet to player's y position.
                if(bullets[bullet_header]->position.y != player->position.y && !playerShot){
                    if(bullets[bullet_header]->position.y <= player->position.y + player->size.y/2) {
                        bullets[bullet_header]->position.y += bullets[bullet_header]->velocity.y * elapsed;
                        modelBullet.Translate(0.0, bullets[bullet_header]->velocity.y * elapsed, 0.0);
                    }else if(bullets[bullet_header]->position.y >= player->position.y && !bullets[bullet_header]->dead) {
                        bullets[bullet_header]->position.y -= bullets[bullet_header]->velocity.y * elapsed;
                        modelBullet.Translate(0.0, -bullets[bullet_header]->velocity.x * elapsed, 0.0);
                    }
                }
                
                //Checks if the bullet had been shot. If it has then it will increase its y value until it hits the screen's top border. When it hits the top border, the bullet will return to the players x & y position.
                if(playerShot) {
                    bullets[bullet_header]->sprite.Draw(&program, 0.5, 0.5, 0.0);
                    bullets[bullet_header]->dead = false;
                    bullets[bullet_header]->position.y += bullets[bullet_header]->velocity.y * elapsed + 0.1;
                    modelBullet.Translate(0.0, bullets[bullet_header]->velocity.y * elapsed + 0.1, 0.0);
                }
                
                if(bullets[bullet_header]->position.y + bullets[bullet_header]->size.y >= 2.0) {
                    playerShot = false;
                }
            
            //Bullet Enemy Collision testing
                if(!collision(*enemiesRow1[0], *bullets[bullet_header]) && !enemiesRow1[0]->dead) {
                    enemiesRow1[0]->dead = true;
                    score += 100;
                    playerShot = false;
                }else if(!collision(*enemiesRow1[1], *bullets[bullet_header]) && !enemiesRow1[1]->dead) {
                    enemiesRow1[1]->dead = true;
                    score += 100;
                    playerShot = false;
                }else if(!collision(*enemiesRow1[2], *bullets[bullet_header]) && !enemiesRow1[2]->dead) {
                    enemiesRow1[2]->dead = true;
                    score += 100;
                    playerShot = false;
                }else if(!collision(*enemiesRow1[3], *bullets[bullet_header]) && !enemiesRow1[3]->dead) {
                    enemiesRow1[3]->dead = true;
                    score += 100;
                    playerShot = false;
                }
                else if(!collision(*enemiesRow1[4], *bullets[bullet_header]) && !enemiesRow1[4]->dead) {
                    enemiesRow1[4]->dead = true;
                    score += 100;
                    playerShot = false;
                }else if(!collision(*enemiesRow1[5], *bullets[bullet_header]) && !enemiesRow1[5]->dead) {
                    enemiesRow1[5]->dead = true;
                    score += 100;
                    playerShot = false;
                }else if(!collision(*enemiesRow1[6], *bullets[bullet_header]) && !enemiesRow1[6]->dead) {
                    enemiesRow1[6]->dead = true;
                    score += 100;
                    playerShot = false;
                }else if(!collision(*enemiesRow2[0], *bullets[bullet_header]) && !enemiesRow2[0]->dead) {
                    enemiesRow2[0]->dead = true;
                    score += 100;
                    playerShot = false;
                }else if(!collision(*enemiesRow2[1], *bullets[bullet_header]) && !enemiesRow2[1]->dead) {
                    enemiesRow2[1]->dead = true;
                    score += 100;
                    playerShot = false;
                }else if(!collision(*enemiesRow2[2], *bullets[bullet_header]) && !enemiesRow2[2]->dead) {
                    enemiesRow2[2]->dead = true;
                    score += 100;
                    playerShot = false;
                }else if(!collision(*enemiesRow2[3], *bullets[bullet_header]) && !enemiesRow2[3]->dead) {
                    enemiesRow2[3]->dead = true;
                    score += 100;
                    playerShot = false;
                }else if(!collision(*enemiesRow2[4], *bullets[bullet_header]) && !enemiesRow2[4]->dead) {
                    enemiesRow2[4]->dead = true;
                    score += 100;
                    playerShot = false;
                }else if(!collision(*enemiesRow2[5], *bullets[bullet_header]) && !enemiesRow2[5]->dead) {
                    enemiesRow2[5]->dead = true;
                    score += 100;
                    playerShot = false;
                }else if(!collision(*enemiesRow2[6], *bullets[bullet_header]) && !enemiesRow2[6]->dead) {
                    enemiesRow2[6]->dead = true;
                    score += 100;
                    playerShot = false;
                }else if(!collision(*enemiesRow3[0], *bullets[bullet_header]) && !enemiesRow3[0]->dead) {
                    enemiesRow3[0]->dead = true;
                    score += 100;
                    playerShot = false;
                }else if(!collision(*enemiesRow3[1], *bullets[bullet_header]) && !enemiesRow3[1]->dead) {
                    enemiesRow3[1]->dead = true;
                    score += 100;
                    playerShot = false;
                }else if(!collision(*enemiesRow3[2], *bullets[bullet_header]) && !enemiesRow3[2]->dead) {
                    enemiesRow3[2]->dead = true;
                    score += 100;
                    playerShot = false;
                }else if(!collision(*enemiesRow3[3], *bullets[bullet_header]) && !enemiesRow3[3]->dead) {
                    enemiesRow3[3]->dead = true;
                    score += 100;
                    playerShot = false;
                }else if(!collision(*enemiesRow3[4], *bullets[bullet_header]) && !enemiesRow3[4]->dead) {
                    enemiesRow3[4]->dead = true;
                    score += 100;
                    playerShot = false;
                }else if(!collision(*enemiesRow3[5], *bullets[bullet_header]) && !enemiesRow3[5]->dead) {
                    enemiesRow3[5]->dead = true;
                    score += 100;
                    playerShot = false;
                }else if(!collision(*enemiesRow3[6], *bullets[bullet_header]) && !enemiesRow3[6]->dead) {
                    enemiesRow3[6]->dead = true;
                    score += 100;
                    playerShot = false;
                }
                
        //Enemy Model
            program.setModelMatrix(modelEnemyRow1);

            
            //Row 1 draw
                for(int q = 0; q < enemiesRow1.size(); q++) {
                    if(enemiesRow1[q]->dead) {
                    //Do nothing
                    }
                    else {
                        enemiesRow1[q]->sprite.Draw(&program, 1.0, 1.0, q);
                    }
                }
                
            //Row 1 start movement
                if(start1Moving) {
                    enemiesRow1[0]->position.x += enemiesRow1[0]->velocity.x * elapsed;
                    enemiesRow1[1]->position.x += enemiesRow1[1]->velocity.x * elapsed;
                    enemiesRow1[2]->position.x += enemiesRow1[2]->velocity.x * elapsed;
                    enemiesRow1[3]->position.x += enemiesRow1[3]->velocity.x * elapsed;
                    enemiesRow1[4]->position.x += enemiesRow1[4]->velocity.x * elapsed;
                    enemiesRow1[5]->position.x += enemiesRow1[5]->velocity.x * elapsed;
                    enemiesRow1[6]->position.x += enemiesRow1[6]->velocity.x * elapsed;
                    modelEnemyRow1.Translate(enemiesRow1[6]->velocity.x * elapsed, 0.0, 0.0);
                }
                
            //Row 1 conditions
                if(enemiesRow1[6]->position.x + enemiesRow1[6]->size.x/2 >= 3.55) {
                    row1HitLeft = false;
                    row1HitRight = true;
                    start1Moving = false;
                }else if(enemiesRow1[0]->position.x - enemiesRow1[0]->size.x/2 <= -3.55){
                    row1HitLeft = true;
                    row1HitRight = false;
                }
                
            //Row 1 movement
                if(row1HitLeft && !row1HitRight) {
                    enemiesRow1[0]->position.x += enemiesRow1[0]->velocity.x * elapsed;
                    enemiesRow1[1]->position.x += enemiesRow1[1]->velocity.x * elapsed;
                    enemiesRow1[2]->position.x += enemiesRow1[2]->velocity.x * elapsed;
                    enemiesRow1[3]->position.x += enemiesRow1[3]->velocity.x * elapsed;
                    enemiesRow1[4]->position.x += enemiesRow1[4]->velocity.x * elapsed;
                    enemiesRow1[5]->position.x += enemiesRow1[5]->velocity.x * elapsed;
                    enemiesRow1[6]->position.x += enemiesRow1[6]->velocity.x * elapsed;
                    modelEnemyRow1.Translate(enemiesRow1[6]->velocity.x * elapsed, 0.0, 0.0f);
                }else if(!row1HitLeft && row1HitRight) {
                    enemiesRow1[0]->position.x -= enemiesRow1[0]->velocity.x * elapsed;
                    enemiesRow1[1]->position.x -= enemiesRow1[1]->velocity.x * elapsed;
                    enemiesRow1[2]->position.x -= enemiesRow1[2]->velocity.x * elapsed;
                    enemiesRow1[3]->position.x -= enemiesRow1[3]->velocity.x * elapsed;
                    enemiesRow1[4]->position.x -= enemiesRow1[4]->velocity.x * elapsed;
                    enemiesRow1[5]->position.x -= enemiesRow1[5]->velocity.x * elapsed;
                    enemiesRow1[6]->position.x -= enemiesRow1[6]->velocity.x * elapsed;
                    modelEnemyRow1.Translate(-enemiesRow1[6]->velocity.x * elapsed, 0.0f, 0.0f);
                }

            //Row 2 draw
                program.setModelMatrix(modelEnemyRow2);
                for(int q = 0; q < enemiesRow2.size(); q++) {
                    if(enemiesRow2[q]->dead) {
                        //Do nothing
                    }
                    else {
                        enemiesRow2[q]->sprite.Draw(&program, 1.0f, 1.0f, q);
                    }
                }
                
            //Row 2 start movement
                if(start2Moving) {
                    enemiesRow2[0]->position.x += enemiesRow2[0]->velocity.x * elapsed;
                    enemiesRow2[1]->position.x += enemiesRow2[1]->velocity.x * elapsed;
                    enemiesRow2[2]->position.x += enemiesRow2[2]->velocity.x * elapsed;
                    enemiesRow2[3]->position.x += enemiesRow2[3]->velocity.x * elapsed;
                    enemiesRow2[4]->position.x += enemiesRow2[4]->velocity.x * elapsed;
                    enemiesRow2[5]->position.x += enemiesRow2[5]->velocity.x * elapsed;
                    enemiesRow2[6]->position.x += enemiesRow2[6]->velocity.x * elapsed;
                    modelEnemyRow2.Translate(enemiesRow2[0]->velocity.x * elapsed, 0.0, 0.0);
                    
                }
                
            //Row 2 conditions
                if(enemiesRow2[6]->position.x + enemiesRow2[6]->size.x/2 >= 3.55) {
                    start2Moving = false;
                    row2HitLeft = false;
                    row2HitRight = true;
                }else if(enemiesRow2[0]->position.x - enemiesRow2[0]->size.x/2 <= -3.55){
                    row2HitLeft = true;
                    row2HitRight = false;
                }
                
            //Row 2 movement
                if(row2HitLeft && !row2HitRight) {
                    enemiesRow2[0]->position.x += enemiesRow2[0]->velocity.x * elapsed;
                    enemiesRow2[1]->position.x += enemiesRow2[1]->velocity.x * elapsed;
                    enemiesRow2[2]->position.x += enemiesRow2[2]->velocity.x * elapsed;
                    enemiesRow2[3]->position.x += enemiesRow2[3]->velocity.x * elapsed;
                    enemiesRow2[4]->position.x += enemiesRow2[4]->velocity.x * elapsed;
                    enemiesRow2[5]->position.x += enemiesRow2[5]->velocity.x * elapsed;
                    enemiesRow2[6]->position.x += enemiesRow2[6]->velocity.x * elapsed;
                    modelEnemyRow2.Translate(enemiesRow2[6]->velocity.x * elapsed, 0.0, 0.0f);
                }else if(!row2HitLeft && row2HitRight) {
                    enemiesRow2[0]->position.x -= enemiesRow2[0]->velocity.x * elapsed;
                    enemiesRow2[1]->position.x -= enemiesRow2[1]->velocity.x * elapsed;
                    enemiesRow2[2]->position.x -= enemiesRow2[2]->velocity.x * elapsed;
                    enemiesRow2[3]->position.x -= enemiesRow2[3]->velocity.x * elapsed;
                    enemiesRow2[4]->position.x -= enemiesRow2[4]->velocity.x * elapsed;
                    enemiesRow2[5]->position.x -= enemiesRow2[5]->velocity.x * elapsed;
                    enemiesRow2[6]->position.x -= enemiesRow2[6]->velocity.x * elapsed;
                    modelEnemyRow2.Translate(-enemiesRow2[0]->velocity.x * elapsed, 0.0f, 0.0f);
                }
                
            //Row 3 draw
                program.setModelMatrix(modelEnemyRow3);
                for(int q = 0; q < enemiesRow3.size(); q++) {
                    if(enemiesRow3[q]->dead) {
                        //Do nothing
                    }
                    else {
                        enemiesRow3[q]->sprite.Draw(&program, 1.0f, 1.0f, q);
                    }
                }
            //Row 3 Conditions
                if(start3Moving) {
                    enemiesRow3[0]->position.x += enemiesRow3[0]->velocity.x * elapsed;
                    enemiesRow3[1]->position.x += enemiesRow3[1]->velocity.x * elapsed;
                    enemiesRow3[2]->position.x += enemiesRow3[2]->velocity.x * elapsed;
                    enemiesRow3[3]->position.x += enemiesRow3[3]->velocity.x * elapsed;
                    enemiesRow3[4]->position.x += enemiesRow3[4]->velocity.x * elapsed;
                    enemiesRow3[5]->position.x += enemiesRow3[5]->velocity.x * elapsed;
                    enemiesRow3[6]->position.x += enemiesRow3[6]->velocity.x * elapsed;
                    modelEnemyRow3.Translate(enemiesRow3[6]->velocity.x * elapsed, 0.0, 0.0);
                }
                if(enemiesRow3[6]->position.x + enemiesRow3[6]->size.x/2 >= 3.55) {
                    row3HitLeft = false;
                    row3HitRight = true;
                    start3Moving = false;
                }else if(enemiesRow3[0]->position.x - enemiesRow3[0]->size.x/2 <= -3.55){
                    row3HitLeft = true;
                    row3HitRight = false;
                }
                
            //Row 3 Movement
                if(row3HitLeft && !row3HitRight) {
                    enemiesRow3[0]->position.x += enemiesRow3[0]->velocity.x * elapsed;
                    enemiesRow3[1]->position.x += enemiesRow3[1]->velocity.x * elapsed;
                    enemiesRow3[2]->position.x += enemiesRow3[2]->velocity.x * elapsed;
                    enemiesRow3[3]->position.x += enemiesRow3[3]->velocity.x * elapsed;
                    enemiesRow3[4]->position.x += enemiesRow3[4]->velocity.x * elapsed;
                    enemiesRow3[5]->position.x += enemiesRow3[5]->velocity.x * elapsed;
                    enemiesRow3[6]->position.x += enemiesRow3[6]->velocity.x * elapsed;
                    modelEnemyRow3.Translate(enemiesRow3[0]->velocity.x * elapsed, 0.0, 0.0f);
                }else if(!row3HitLeft && row3HitRight) {
                    enemiesRow3[0]->position.x -= enemiesRow3[0]->velocity.x * elapsed;
                    enemiesRow3[1]->position.x -= enemiesRow3[1]->velocity.x * elapsed;
                    enemiesRow3[2]->position.x -= enemiesRow3[2]->velocity.x * elapsed;
                    enemiesRow3[3]->position.x -= enemiesRow3[3]->velocity.x * elapsed;
                    enemiesRow3[4]->position.x -= enemiesRow3[4]->velocity.x * elapsed;
                    enemiesRow3[5]->position.x -= enemiesRow3[5]->velocity.x * elapsed;
                    enemiesRow3[6]->position.x -= enemiesRow3[6]->velocity.x * elapsed;
                    modelEnemyRow3.Translate(-enemiesRow3[6]->velocity.x * elapsed, 0.0f, 0.0f);
                }
                
            //Enemy Player Collision testing
                if(!collision(*player, *enemiesRow1[0]) && !enemiesRow1[0]->dead) {
                    player->dead = true;
                    state = STATE_GAME_END;
                }else if(!collision(*player, *enemiesRow1[1]) && !enemiesRow1[1]->dead) {
                    player->dead = true;
                    state = STATE_GAME_END;
                }else if(!collision(*player, *enemiesRow1[2]) && !enemiesRow1[2]->dead) {
                    player->dead = true;
                    state = STATE_GAME_END;
                }else if(!collision(*player, *enemiesRow1[3]) && !enemiesRow1[3]->dead) {
                    player->dead = true;
                    state = STATE_GAME_END;
                }else if(!collision(*player, *enemiesRow1[4]) && !enemiesRow1[4]->dead) {
                    player->dead = true;
                    state = STATE_GAME_END;
                }else if(!collision(*player, *enemiesRow1[5]) && !enemiesRow1[5]->dead) {
                    player->dead = true;
                    state = STATE_GAME_END;
                }else if(!collision(*player, *enemiesRow1[6]) && !enemiesRow1[6]->dead) {
                    player->dead = true;
                    state = STATE_GAME_END;
                }else if(!collision(*player, *enemiesRow2[0]) && !enemiesRow2[0]->dead) {
                    player->dead = true;
                    state = STATE_GAME_END;
                }else if(!collision(*player, *enemiesRow2[1]) && !enemiesRow2[1]->dead) {
                    player->dead = true;
                    state = STATE_GAME_END;
                }else if(!collision(*player, *enemiesRow2[2]) && !enemiesRow2[2]->dead) {
                    player->dead = true;
                    state = STATE_GAME_END;
                }else if(!collision(*player, *enemiesRow2[3]) && !enemiesRow2[3]->dead) {
                    player->dead = true;
                    state = STATE_GAME_END;
                }else if(!collision(*player, *enemiesRow2[4]) && !enemiesRow2[4]->dead) {
                    player->dead = true;
                    state = STATE_GAME_END;
                }else if(!collision(*player, *enemiesRow2[5]) && !enemiesRow2[5]->dead) {
                    player->dead = true;
                    state = STATE_GAME_END;
                }else if(!collision(*player, *enemiesRow2[6]) && !enemiesRow2[6]->dead) {
                    player->dead = true;
                    state = STATE_GAME_END;
                }else if(!collision(*player, *enemiesRow3[0]) && !enemiesRow3[0]->dead) {
                    player->dead = true;
                    state = STATE_GAME_END;
                }else if(!collision(*player, *enemiesRow3[1]) && !enemiesRow3[1]->dead) {
                    player->dead = true;
                    state = STATE_GAME_END;
                }else if(!collision(*player, *enemiesRow3[2]) && !enemiesRow3[2]->dead) {
                    player->dead = true;
                    state = STATE_GAME_END;
                }else if(!collision(*player, *enemiesRow3[3]) && !enemiesRow3[3]->dead) {
                    player->dead = true;
                    state = STATE_GAME_END;
                }else if(!collision(*player, *enemiesRow3[4]) && !enemiesRow3[4]->dead) {
                    player->dead = true;
                    state = STATE_GAME_END;
                }else if(!collision(*player, *enemiesRow3[5]) && !enemiesRow3[5]->dead) {
                    player->dead = true;
                    state = STATE_GAME_END;
                }else if(!collision(*player, *enemiesRow3[6]) && !enemiesRow3[6]->dead) {
                    player->dead = true;
                    state = STATE_GAME_END;
                }

            //Score model
                program.setModelMatrix(modelScore);
                DrawText(&program, fontTexture, "Score:" + std::to_string(score), 0.2, 0.0);
            
                if(score == 2100) state = STATE_GAME_END;
            
            break;
                
            case STATE_GAME_END:
                
                modelScore.identity();
                
                if(score == 2100) {
                    modelScore.Translate(-0.5, 0.0, 0.0);
                    program.setModelMatrix(modelScore);
                    DrawText(&program, fontTexture, "Win!", 0.5, 0.0);
                }else {
                    modelScore.Translate(-1.2, 0.0, 0.0);
                    program.setModelMatrix(modelScore);
                    DrawText(&program, fontTexture, "Game Over!", 0.3, 0.0);
                }
                
            break;
        }
        
        SDL_GL_SwapWindow(displayWindow);
    }
    std::cout << "Delete Player, Bullets, & Enemies\n";
    delete player;
    //delete bullet;
    
    for(int i = 0; i < 2; i++){
        delete bullets[i];
    }
    
    for(int d = 0; d < enemiesRow1.size(); d++) {
        delete enemiesRow1.back();
        enemiesRow1.pop_back();
    }
    
    for(int d = 0; d < enemiesRow2.size(); d++) {
        delete enemiesRow2.back();
        enemiesRow2.pop_back();
    }
    
    for(int d = 0; d < enemiesRow2.size(); d++) {
        delete enemiesRow2.back();
        enemiesRow2.pop_back();
    }
    SDL_Quit();
    return 0;
}
